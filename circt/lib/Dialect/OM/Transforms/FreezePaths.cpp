//===- FreezePaths.cpp - Freeze Paths pass ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Contains the definitions of the OM freeze paths pass.
//
//===----------------------------------------------------------------------===//

#include "PassDetails.h"
#include "circt/Dialect/HW/HWInstanceGraph.h"
#include "circt/Dialect/HW/HWOps.h"
#include "circt/Dialect/HW/InnerSymbolTable.h"
#include "circt/Dialect/OM/OMAttributes.h"
#include "circt/Dialect/OM/OMOps.h"
#include "circt/Dialect/OM/OMPasses.h"

using namespace circt;
using namespace om;

namespace {
struct PathVisitor {
  PathVisitor(hw::InstanceGraph &instanceGraph, hw::InnerRefNamespace &irn)
      : instanceGraph(instanceGraph), irn(irn) {}

  StringAttr field;
  LogicalResult processPath(Location loc, hw::HierPathOp hierPathOp,
                            PathAttr &targetPath, StringAttr &bottomModule,
                            StringAttr &component, StringAttr &field);
  LogicalResult process(BasePathCreateOp pathOp);
  LogicalResult process(PathCreateOp pathOp);
  LogicalResult process(EmptyPathOp pathOp);
  LogicalResult run(ModuleOp module);
  hw::InstanceGraph &instanceGraph;
  hw::InnerRefNamespace &irn;
};
} // namespace

static LogicalResult getAccessPath(Location loc, Type type, size_t fieldId,
                                   StringAttr &result) {
  SmallString<64> field;
  while (fieldId) {
    if (auto aliasType = dyn_cast<hw::TypeAliasType>(type))
      type = aliasType.getCanonicalType();
    if (auto structType = dyn_cast<hw::StructType>(type)) {
      auto index = structType.getIndexForFieldID(fieldId);
      auto &element = structType.getElements()[index];
      field.push_back('.');
      llvm::append_range(field, element.name.getValue());
      type = element.type;
      fieldId -= structType.getFieldID(index);
    } else if (auto arrayType = dyn_cast<hw::ArrayType>(type)) {
      auto index = arrayType.getIndexForFieldID(fieldId);
      field.push_back('[');
      Twine(index).toVector(field);
      field.push_back(']');
      type = arrayType.getElementType();
      fieldId -= arrayType.getFieldID(index);
    } else {
      return emitError(loc) << "can't create access path with fieldID "
                            << fieldId << " in type " << type;
    }
  }
  result = StringAttr::get(loc->getContext(), field);
  return success();
}

LogicalResult PathVisitor::processPath(Location loc, hw::HierPathOp hierPathOp,
                                       PathAttr &targetPath,
                                       StringAttr &bottomModule,
                                       StringAttr &component,
                                       StringAttr &field) {
  auto *context = hierPathOp->getContext();
  // Look up the associated HierPathOp.
  auto namepath = hierPathOp.getNamepathAttr().getValue();
  SmallVector<PathElement> modules;

  // Process the final target first.
  auto &end = namepath.back();
  if (auto innerRef = dyn_cast<hw::InnerRefAttr>(end)) {
    auto target = irn.lookup(innerRef);
    if (target.isPort()) {
      // We are targeting the port of a module.
      auto module = cast<hw::HWModuleLike>(target.getOp());
      auto index = target.getPort();
      bottomModule = module.getModuleNameAttr();
      component = StringAttr::get(context, module.getPortName(index));
      auto loc = module.getPortLoc(index);
      auto type = module.getPortTypes()[index];
      if (failed(getAccessPath(loc, type, target.getField(), field)))
        return failure();
    } else {
      auto *op = target.getOp();
      assert(op && "innerRef should be targeting something");
      // Get the current module.
      auto currentModule = innerRef.getModule();
      // Get the verilog name of the target.
      auto verilogName = op->getAttrOfType<StringAttr>("hw.verilogName");
      if (!verilogName) {
        auto diag = emitError(loc, "component does not have verilog name");
        diag.attachNote(op->getLoc()) << "component here";
        return diag;
      }
      // If this is our inner ref pair: [Foo::bar]
      // if "bar" is an instance, modules = [Foo::bar], bottomModule = Bar.
      // if "bar" is a wire, modules = [], bottomModule = Foo, component = bar.
      if (auto inst = dyn_cast<hw::HWInstanceLike>(op)) {
        // We are targeting an instance.
        modules.emplace_back(currentModule, verilogName);
        bottomModule = inst.getReferencedModuleNameAttr();
        component = StringAttr::get(context, "");
        field = StringAttr::get(context, "");
      } else {
        // We are targeting a regular component.
        bottomModule = currentModule;
        component = verilogName;
        auto innerSym = cast<hw::InnerSymbolOpInterface>(op);
        auto value = innerSym.getTargetResult();
        if (failed(getAccessPath(value.getLoc(), value.getType(),
                                 target.getField(), field)))
          return failure();
      }
    }
  } else {
    // We are targeting a module.
    auto symbolRef = cast<FlatSymbolRefAttr>(end);
    bottomModule = symbolRef.getAttr();
    component = StringAttr::get(context, "");
    field = StringAttr::get(context, "");
  }

  // Process the rest of the hierarchical path.
  for (auto attr : llvm::reverse(namepath.drop_back())) {
    auto innerRef = cast<hw::InnerRefAttr>(attr);
    auto target = irn.lookup(innerRef);
    assert(target.isOpOnly() && "can't target a port the middle of a namepath");
    auto *op = target.getOp();
    // Get the verilog name of the target.
    auto verilogName = op->getAttrOfType<StringAttr>("hw.verilogName");
    if (!verilogName) {
      auto diag = emitError(loc, "component does not have verilog name");
      diag.attachNote(op->getLoc()) << "component here";
      return diag;
    }
    modules.emplace_back(innerRef.getModule(), verilogName);
  }

  auto topRef = namepath.front();

  auto topModule = isa<hw::InnerRefAttr>(topRef)
                       ? cast<hw::InnerRefAttr>(topRef).getModule()
                       : cast<FlatSymbolRefAttr>(topRef).getAttr();

  // Handle the modules not present in the path.
  auto *node = instanceGraph.lookup(topModule);
  while (!node->noUses()) {
    auto module = node->getModule<hw::HWModuleLike>();

    // If the module is public, stop here.
    if (module.isPublic())
      break;

    if (!node->hasOneUse()) {
      auto diag = emitError(loc) << "unable to uniquely resolve target "
                                    "due to multiple instantiation";
      for (auto *use : node->uses()) {
        if (auto *op = use->getInstance<Operation *>())
          diag.attachNote(op->getLoc()) << "instance here";
        else {
          auto module = node->getModule();
          diag.attachNote(module->getLoc()) << "module marked public";
        }
      }
      return diag;
    }
    // Get the single instance of this module.
    auto *record = *node->usesBegin();
    auto *inst = record->getInstance<Operation *>();
    // If the instance is external, just break here.
    if (!inst)
      break;
    // Get the verilog name of the instance.
    auto verilogName = inst->getAttrOfType<StringAttr>("hw.verilogName");
    if (!verilogName)
      return inst->emitError("component does not have verilog name");
    node = record->getParent();
    modules.emplace_back(node->getModule().getModuleNameAttr(), verilogName);
  }

  // Create the target path.
  targetPath = PathAttr::get(context, llvm::to_vector(llvm::reverse(modules)));
  return success();
}

LogicalResult PathVisitor::process(PathCreateOp path) {
  auto hierPathOp =
      irn.symTable.lookup<hw::HierPathOp>(path.getTargetAttr().getAttr());
  PathAttr targetPath;
  StringAttr bottomModule;
  StringAttr ref;
  StringAttr field;
  if (failed(processPath(path.getLoc(), hierPathOp, targetPath, bottomModule,
                         ref, field)))
    return failure();

  // Replace the old path operation.
  OpBuilder builder(path);
  auto frozenPath = builder.create<FrozenPathCreateOp>(
      path.getLoc(), path.getTargetKindAttr(), path->getOperand(0), targetPath,
      bottomModule, ref, field);
  path.replaceAllUsesWith(frozenPath.getResult());
  path->erase();

  return success();
}

LogicalResult PathVisitor::process(BasePathCreateOp path) {
  auto hierPathOp =
      irn.symTable.lookup<hw::HierPathOp>(path.getTargetAttr().getAttr());
  PathAttr targetPath;
  StringAttr bottomModule;
  StringAttr ref;
  StringAttr field;
  if (failed(processPath(path.getLoc(), hierPathOp, targetPath, bottomModule,
                         ref, field)))
    return failure();

  if (!ref.empty())
    return path->emitError("basepath must target an instance");
  if (!field.empty())
    return path->emitError("basepath must not target a field");

  // Replace the old path operation.
  OpBuilder builder(path);
  auto frozenPath = builder.create<FrozenBasePathCreateOp>(
      path.getLoc(), path->getOperand(0), targetPath);
  path.replaceAllUsesWith(frozenPath.getResult());
  path->erase();

  return success();
}

LogicalResult PathVisitor::process(EmptyPathOp path) {
  OpBuilder builder(path);
  auto frozenPath = builder.create<FrozenEmptyPathOp>(path.getLoc());
  path.replaceAllUsesWith(frozenPath.getResult());
  path->erase();
  return success();
}

LogicalResult PathVisitor::run(ModuleOp module) {
  auto frozenBasePathType = FrozenBasePathType::get(module.getContext());
  auto frozenPathType = FrozenPathType::get(module.getContext());
  auto updatePathType = [&](Value value) {
    if (isa<BasePathType>(value.getType()))
      value.setType(frozenBasePathType);
    if (isa<PathType>(value.getType()))
      value.setType(frozenPathType);
  };
  for (auto classLike : module.getOps<ClassLike>()) {
    // Transform PathType block argument to FrozenPathType.
    for (auto arg : classLike.getBodyBlock()->getArguments())
      updatePathType(arg);
    auto result = classLike->walk([&](Operation *op) -> WalkResult {
      if (auto basePath = dyn_cast<BasePathCreateOp>(op)) {
        if (failed(process(basePath)))
          return WalkResult::interrupt();
      } else if (auto path = dyn_cast<PathCreateOp>(op)) {
        if (failed(process(path)))
          return WalkResult::interrupt();
      } else if (auto path = dyn_cast<EmptyPathOp>(op)) {
        if (failed(process(path)))
          return WalkResult::interrupt();
      }
      return WalkResult::advance();
    });
    if (result.wasInterrupted())
      return failure();
  }
  return success();
}

namespace {
struct FreezePathsPass : public FreezePathsBase<FreezePathsPass> {
  void runOnOperation() override;
};
} // namespace

void FreezePathsPass::runOnOperation() {
  auto module = getOperation();
  auto &instanceGraph = getAnalysis<hw::InstanceGraph>();
  auto &symbolTable = getAnalysis<SymbolTable>();
  hw::InnerSymbolTableCollection collection(module);
  hw::InnerRefNamespace irn{symbolTable, collection};
  if (failed(PathVisitor(instanceGraph, irn).run(module)))
    signalPassFailure();
}

std::unique_ptr<mlir::Pass> circt::om::createFreezePathsPass() {
  return std::make_unique<FreezePathsPass>();
}
