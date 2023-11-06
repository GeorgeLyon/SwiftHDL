//===- LowerSeqToSV.cpp - Seq to SV lowering ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This transform translate Seq ops to SV.
//
//===----------------------------------------------------------------------===//

#include "circt/Conversion/SeqToSV.h"
#include "../PassDetail.h"
#include "FirMemLowering.h"
#include "FirRegLowering.h"
#include "circt/Dialect/Comb/CombOps.h"
#include "circt/Dialect/HW/ConversionPatterns.h"
#include "circt/Dialect/HW/HWAttributes.h"
#include "circt/Dialect/HW/HWOps.h"
#include "circt/Dialect/HW/HWTypes.h"
#include "circt/Dialect/SV/SVAttributes.h"
#include "circt/Dialect/SV/SVOps.h"
#include "circt/Dialect/Seq/SeqOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/ImplicitLocOpBuilder.h"
#include "mlir/IR/Threading.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/ADT/IntervalMap.h"
#include "llvm/ADT/TypeSwitch.h"

#define DEBUG_TYPE "lower-seq-to-sv"

using namespace circt;
using namespace seq;
using hw::HWModuleOp;
using llvm::MapVector;

namespace {
#define GEN_PASS_DEF_LOWERSEQTOSV
#include "circt/Conversion/Passes.h.inc"

struct SeqToSVPass : public impl::LowerSeqToSVBase<SeqToSVPass> {
  void runOnOperation() override;
  using LowerSeqToSVBase<SeqToSVPass>::lowerToAlwaysFF;
  using LowerSeqToSVBase<SeqToSVPass>::disableRegRandomization;
  using LowerSeqToSVBase<SeqToSVPass>::emitSeparateAlwaysBlocks;
  using LowerSeqToSVBase<SeqToSVPass>::LowerSeqToSVBase;
  using LowerSeqToSVBase<SeqToSVPass>::numSubaccessRestored;
};
} // anonymous namespace

namespace {
/// Lower CompRegOp to `sv.reg` and `sv.alwaysff`. Use a posedge clock and
/// synchronous reset.
template <typename OpTy>
class CompRegLower : public OpConversionPattern<OpTy> {
public:
  CompRegLower(TypeConverter &typeConverter, MLIRContext *context,
               bool lowerToAlwaysFF)
      : OpConversionPattern<OpTy>(typeConverter, context),
        lowerToAlwaysFF(lowerToAlwaysFF) {}

  using OpAdaptor = typename OpConversionPattern<OpTy>::OpAdaptor;

  LogicalResult
  matchAndRewrite(OpTy reg, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    Location loc = reg.getLoc();

    auto regTy =
        ConversionPattern::getTypeConverter()->convertType(reg.getType());

    auto svReg = rewriter.create<sv::RegOp>(loc, regTy, reg.getNameAttr(),
                                            reg.getInnerSymAttr(),
                                            reg.getPowerOnValue());
    svReg->setDialectAttrs(reg->getDialectAttrs());

    circt::sv::setSVAttributes(svReg, circt::sv::getSVAttributes(reg));

    auto regVal = rewriter.create<sv::ReadInOutOp>(loc, svReg);

    auto assignValue = [&] {
      createAssign(rewriter, reg.getLoc(), svReg, reg);
    };
    auto assignReset = [&] {
      rewriter.create<sv::PAssignOp>(loc, svReg, adaptor.getResetValue());
    };

    if (adaptor.getReset() && adaptor.getResetValue()) {
      if (lowerToAlwaysFF) {
        rewriter.create<sv::AlwaysFFOp>(
            loc, sv::EventControl::AtPosEdge, adaptor.getClk(),
            ResetType::SyncReset, sv::EventControl::AtPosEdge,
            adaptor.getReset(), assignValue, assignReset);
      } else {
        rewriter.create<sv::AlwaysOp>(
            loc, sv::EventControl::AtPosEdge, adaptor.getClk(), [&] {
              rewriter.create<sv::IfOp>(loc, adaptor.getReset(), assignReset,
                                        assignValue);
            });
      }
    } else {
      if (lowerToAlwaysFF) {
        rewriter.create<sv::AlwaysFFOp>(loc, sv::EventControl::AtPosEdge,
                                        adaptor.getClk(), assignValue);
      } else {
        rewriter.create<sv::AlwaysOp>(loc, sv::EventControl::AtPosEdge,
                                      adaptor.getClk(), assignValue);
      }
    }

    rewriter.replaceOp(reg, regVal);
    return success();
  }

  // Helper to create an assignment based on the register type.
  void createAssign(ConversionPatternRewriter &rewriter, Location loc,
                    sv::RegOp svReg, OpAdaptor reg) const;

private:
  bool lowerToAlwaysFF;
};

/// Create the assign.
template <>
void CompRegLower<CompRegOp>::createAssign(ConversionPatternRewriter &rewriter,
                                           Location loc, sv::RegOp svReg,
                                           OpAdaptor reg) const {
  rewriter.create<sv::PAssignOp>(loc, svReg, reg.getInput());
}
/// Create the assign inside of an if block.
template <>
void CompRegLower<CompRegClockEnabledOp>::createAssign(
    ConversionPatternRewriter &rewriter, Location loc, sv::RegOp svReg,
    OpAdaptor reg) const {
  rewriter.create<sv::IfOp>(loc, reg.getClockEnable(), [&]() {
    rewriter.create<sv::PAssignOp>(loc, svReg, reg.getInput());
  });
}

// Lower seq.clock_gate to a fairly standard clock gate implementation.
//
class ClockGateLowering : public OpConversionPattern<ClockGateOp> {
public:
  using OpConversionPattern<ClockGateOp>::OpConversionPattern;
  using OpAdaptor = typename OpConversionPattern<ClockGateOp>::OpAdaptor;
  LogicalResult
  matchAndRewrite(ClockGateOp clockGate, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    auto loc = clockGate.getLoc();
    Value clk = adaptor.getInput();

    // enable in
    Value enable = adaptor.getEnable();
    if (auto te = adaptor.getTestEnable())
      enable = rewriter.create<comb::OrOp>(loc, enable, te);

    // Enable latch.
    Value enableLatch = rewriter.create<sv::RegOp>(
        loc, rewriter.getI1Type(), rewriter.getStringAttr("cg_en_latch"));

    // Latch the enable signal using an always @* block.
    rewriter.create<sv::AlwaysOp>(
        loc, llvm::SmallVector<sv::EventControl>{}, llvm::SmallVector<Value>{},
        [&]() {
          rewriter.create<sv::IfOp>(
              loc, comb::createOrFoldNot(loc, clk, rewriter), [&]() {
                rewriter.create<sv::PAssignOp>(loc, enableLatch, enable);
              });
        });

    // Create the gated clock signal.
    Value gclk = rewriter.create<comb::AndOp>(
        loc, clk, rewriter.create<sv::ReadInOutOp>(loc, enableLatch));
    clockGate.replaceAllUsesWith(gclk);
    rewriter.eraseOp(clockGate);
    return success();
  }
};

// Lower seq.clock_mux to a `comb.mux` op
//
class ClockMuxLowering : public OpConversionPattern<ClockMuxOp> {
public:
  using OpConversionPattern<ClockMuxOp>::OpConversionPattern;
  using OpConversionPattern<ClockMuxOp>::OpAdaptor;

  LogicalResult
  matchAndRewrite(ClockMuxOp clockMux, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<comb::MuxOp>(clockMux, adaptor.getCond(),
                                             adaptor.getTrueClock(),
                                             adaptor.getFalseClock(), true);
    return success();
  }
};

/// Map `seq.clock` to `i1`.
struct SeqToSVTypeConverter : public TypeConverter {
  SeqToSVTypeConverter() {
    addConversion([&](Type type) { return type; });
    addConversion([&](seq::ClockType type) {
      return IntegerType::get(type.getContext(), 1);
    });
    addConversion([&](hw::StructType structTy) {
      bool changed = false;

      SmallVector<hw::StructType::FieldInfo> newFields;
      for (auto field : structTy.getElements()) {
        auto &newField = newFields.emplace_back();
        newField.name = field.name;
        newField.type = convertType(field.type);
        if (field.type != newField.type)
          changed = true;
      }

      if (!changed)
        return structTy;

      return hw::StructType::get(structTy.getContext(), newFields);
    });
    addConversion([&](hw::ArrayType arrayTy) {
      auto elementTy = arrayTy.getElementType();
      auto newElementTy = convertType(elementTy);
      if (elementTy != newElementTy)
        return hw::ArrayType::get(newElementTy, arrayTy.getNumElements());
      return arrayTy;
    });

    addTargetMaterialization(
        [&](mlir::OpBuilder &builder, mlir::Type resultType,
            mlir::ValueRange inputs,
            mlir::Location loc) -> std::optional<mlir::Value> {
          if (inputs.size() != 1)
            return std::nullopt;
          return inputs[0];
        });

    addSourceMaterialization(
        [&](mlir::OpBuilder &builder, mlir::Type resultType,
            mlir::ValueRange inputs,
            mlir::Location loc) -> std::optional<mlir::Value> {
          if (inputs.size() != 1)
            return std::nullopt;
          return inputs[0];
        });
  }
};

/// Eliminate no-op clock casts.
template <typename T>
class ClockCastLowering : public OpConversionPattern<T> {
public:
  using OpConversionPattern<T>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(T op, typename T::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOp(op, adaptor.getInput());
    return success();
  }
};

// Lower seq.const_clock to `hw.constant`
//
class ClockConstLowering : public OpConversionPattern<ConstClockOp> {
public:
  using OpConversionPattern<ConstClockOp>::OpConversionPattern;
  using OpConversionPattern<ConstClockOp>::OpAdaptor;

  LogicalResult
  matchAndRewrite(ConstClockOp clockConst, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<hw::ConstantOp>(
        clockConst, APInt(1, clockConst.getValue() == ClockConst::High));
    return success();
  }
};

/// Lower `seq.clock_div` to a behavioural clock divider
///
class ClockDividerLowering : public OpConversionPattern<ClockDivider> {
public:
  using OpConversionPattern<ClockDivider>::OpConversionPattern;
  using OpConversionPattern<ClockDivider>::OpAdaptor;

  LogicalResult
  matchAndRewrite(ClockDivider clockDiv, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    Location loc = clockDiv.getLoc();

    Value one;
    if (clockDiv.getPow2()) {
      one = rewriter.create<hw::ConstantOp>(loc, APInt(1, 1));
    }

    Value output = clockDiv.getClockIn();

    SmallVector<Value> regs;
    for (unsigned i = 0; i < clockDiv.getPow2(); ++i) {
      Value reg = rewriter.create<sv::RegOp>(
          loc, rewriter.getI1Type(),
          rewriter.getStringAttr("clock_out_" + std::to_string(i)));
      regs.push_back(reg);

      rewriter.create<sv::AlwaysOp>(
          loc, sv::EventControl::AtPosEdge, output, [&] {
            Value outputVal = rewriter.create<sv::ReadInOutOp>(loc, reg);
            Value inverted = rewriter.create<comb::XorOp>(loc, outputVal, one);
            rewriter.create<sv::BPAssignOp>(loc, reg, inverted);
          });

      output = rewriter.create<sv::ReadInOutOp>(loc, reg);
    }

    if (!regs.empty()) {
      Value zero = rewriter.create<hw::ConstantOp>(loc, APInt(1, 0));
      rewriter.create<sv::InitialOp>(loc, [&] {
        for (Value reg : regs) {
          rewriter.create<sv::BPAssignOp>(loc, reg, zero);
        }
      });
    }

    rewriter.replaceOp(clockDiv, output);
    return success();
  }
};

} // namespace

// NOLINTBEGIN(misc-no-recursion)
static bool isLegalType(Type ty) {
  if (hw::type_isa<ClockType>(ty))
    return false;

  if (auto arrayTy = hw::type_dyn_cast<hw::ArrayType>(ty))
    return isLegalType(arrayTy.getElementType());

  if (auto structTy = hw::type_dyn_cast<hw::StructType>(ty)) {
    for (auto field : structTy.getElements())
      if (!isLegalType(field.type))
        return false;
    return true;
  }

  return true;
}
// NOLINTEND(misc-no-recursion)

static bool isLegalOp(Operation *op) {
  if (auto module = dyn_cast<hw::HWModuleLike>(op)) {
    return llvm::all_of(module.getPortList(), [](hw::PortInfo port) {
      return isLegalType(port.type);
    });
  }
  bool allOperandsLowered = llvm::all_of(
      op->getOperands(), [](auto op) { return isLegalType(op.getType()); });
  bool allResultsLowered = llvm::all_of(op->getResults(), [](auto result) {
    return isLegalType(result.getType());
  });
  return allOperandsLowered && allResultsLowered;
}

void SeqToSVPass::runOnOperation() {
  auto circuit = getOperation();
  MLIRContext *context = &getContext();

  auto modules = llvm::to_vector(circuit.getOps<HWModuleOp>());

  FirMemLowering memLowering(circuit);

  // Identify memories and group them by module.
  auto uniqueMems = memLowering.collectMemories(modules);
  MapVector<HWModuleOp, SmallVector<FirMemLowering::MemoryConfig>> memsByModule;
  for (auto &[config, memOps] : uniqueMems) {
    // Create the `HWModuleGeneratedOp`s for each unique configuration.
    auto genOp = memLowering.createMemoryModule(config, memOps);

    // Group memories by their parent module for parallelism.
    for (auto memOp : memOps) {
      auto parent = memOp->getParentOfType<HWModuleOp>();
      memsByModule[parent].emplace_back(&config, genOp, memOp);
    }
  }

  // Lower memories and registers in modules in parallel.
  mlir::parallelForEach(&getContext(), modules, [&](HWModuleOp module) {
    SeqToSVTypeConverter typeConverter;
    FirRegLowering regLowering(typeConverter, module, disableRegRandomization,
                               emitSeparateAlwaysBlocks);
    regLowering.lower();
    numSubaccessRestored += regLowering.numSubaccessRestored;

    if (auto *it = memsByModule.find(module); it != memsByModule.end())
      memLowering.lowerMemoriesInModule(module, it->second);
  });

  // Mark all ops which can have clock types as illegal.
  SeqToSVTypeConverter typeConverter;
  ConversionTarget target(*context);
  target.addIllegalDialect<SeqDialect>();
  target.markUnknownOpDynamicallyLegal(isLegalOp);

  RewritePatternSet patterns(context);
  patterns.add<CompRegLower<CompRegOp>>(typeConverter, context,
                                        lowerToAlwaysFF);
  patterns.add<CompRegLower<CompRegClockEnabledOp>>(typeConverter, context,
                                                    lowerToAlwaysFF);
  patterns.add<ClockCastLowering<seq::FromClockOp>>(typeConverter, context);
  patterns.add<ClockCastLowering<seq::ToClockOp>>(typeConverter, context);
  patterns.add<ClockGateLowering>(typeConverter, context);
  patterns.add<ClockMuxLowering>(typeConverter, context);
  patterns.add<ClockDividerLowering>(typeConverter, context);
  patterns.add<ClockConstLowering>(typeConverter, context);
  patterns.add<TypeConversionPattern>(typeConverter, context);

  if (failed(applyPartialConversion(circuit, target, std::move(patterns))))
    signalPassFailure();
}

std::unique_ptr<Pass>
circt::createLowerSeqToSVPass(const LowerSeqToSVOptions &options) {
  return std::make_unique<SeqToSVPass>(options);
}
