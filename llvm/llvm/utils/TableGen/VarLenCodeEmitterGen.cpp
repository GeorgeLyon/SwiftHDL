//===- VarLenCodeEmitterGen.cpp - CEG for variable-length insts -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// The CodeEmitterGen component for variable-length instructions.
//
// The basic CodeEmitterGen is almost exclusively designed for fixed-
// length instructions. A good analogy for its encoding scheme is how printf
// works: The (immutable) formatting string represent the fixed values in the
// encoded instruction. Placeholders (i.e. %something), on the other hand,
// represent encoding for instruction operands.
// ```
// printf("1101 %src 1001 %dst", <encoded value for operand `src`>,
//                               <encoded value for operand `dst`>);
// ```
// VarLenCodeEmitterGen in this file provides an alternative encoding scheme
// that works more like a C++ stream operator:
// ```
// OS << 0b1101;
// if (Cond)
//   OS << OperandEncoding0;
// OS << 0b1001 << OperandEncoding1;
// ```
// You are free to concatenate arbitrary types (and sizes) of encoding
// fragments on any bit position, bringing more flexibilities on defining
// encoding for variable-length instructions.
//
// In a more specific way, instruction encoding is represented by a DAG type
// `Inst` field. Here is an example:
// ```
// dag Inst = (descend 0b1101, (operand "$src", 4), 0b1001,
//                     (operand "$dst", 4));
// ```
// It represents the following instruction encoding:
// ```
// MSB                                                     LSB
// 1101<encoding for operand src>1001<encoding for operand dst>
// ```
// For more details about DAG operators in the above snippet, please
// refer to \file include/llvm/Target/Target.td.
//
// VarLenCodeEmitter will convert the above DAG into the same helper function
// generated by CodeEmitter, `MCCodeEmitter::getBinaryCodeForInstr` (except
// for few details).
//
//===----------------------------------------------------------------------===//

#include "VarLenCodeEmitterGen.h"
#include "CodeGenHwModes.h"
#include "CodeGenInstruction.h"
#include "CodeGenTarget.h"
#include "InfoByHwMode.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"

using namespace llvm;

namespace {

class VarLenCodeEmitterGen {
  RecordKeeper &Records;

  DenseMap<Record *, VarLenInst> VarLenInsts;

  // Emit based values (i.e. fixed bits in the encoded instructions)
  void emitInstructionBaseValues(
      raw_ostream &OS,
      ArrayRef<const CodeGenInstruction *> NumberedInstructions,
      CodeGenTarget &Target, int HwMode = -1);

  std::string getInstructionCase(Record *R, CodeGenTarget &Target);
  std::string getInstructionCaseForEncoding(Record *R, Record *EncodingDef,
                                            CodeGenTarget &Target);

public:
  explicit VarLenCodeEmitterGen(RecordKeeper &R) : Records(R) {}

  void run(raw_ostream &OS);
};
} // end anonymous namespace

// Get the name of custom encoder or decoder, if there is any.
// Returns `{encoder name, decoder name}`.
static std::pair<StringRef, StringRef> getCustomCoders(ArrayRef<Init *> Args) {
  std::pair<StringRef, StringRef> Result;
  for (const auto *Arg : Args) {
    const auto *DI = dyn_cast<DagInit>(Arg);
    if (!DI)
      continue;
    const Init *Op = DI->getOperator();
    if (!isa<DefInit>(Op))
      continue;
    // syntax: `(<encoder | decoder> "function name")`
    StringRef OpName = cast<DefInit>(Op)->getDef()->getName();
    if (OpName != "encoder" && OpName != "decoder")
      continue;
    if (!DI->getNumArgs() || !isa<StringInit>(DI->getArg(0)))
      PrintFatalError("expected '" + OpName +
                      "' directive to be followed by a custom function name.");
    StringRef FuncName = cast<StringInit>(DI->getArg(0))->getValue();
    if (OpName == "encoder")
      Result.first = FuncName;
    else
      Result.second = FuncName;
  }
  return Result;
}

VarLenInst::VarLenInst(const DagInit *DI, const RecordVal *TheDef)
    : TheDef(TheDef), NumBits(0U), HasDynamicSegment(false) {
  buildRec(DI);
  for (const auto &S : Segments)
    NumBits += S.BitWidth;
}

void VarLenInst::buildRec(const DagInit *DI) {
  assert(TheDef && "The def record is nullptr ?");

  std::string Op = DI->getOperator()->getAsString();

  if (Op == "ascend" || Op == "descend") {
    bool Reverse = Op == "descend";
    int i = Reverse ? DI->getNumArgs() - 1 : 0;
    int e = Reverse ? -1 : DI->getNumArgs();
    int s = Reverse ? -1 : 1;
    for (; i != e; i += s) {
      const Init *Arg = DI->getArg(i);
      if (const auto *BI = dyn_cast<BitsInit>(Arg)) {
        if (!BI->isComplete())
          PrintFatalError(TheDef->getLoc(),
                          "Expecting complete bits init in `" + Op + "`");
        Segments.push_back({BI->getNumBits(), BI});
      } else if (const auto *BI = dyn_cast<BitInit>(Arg)) {
        if (!BI->isConcrete())
          PrintFatalError(TheDef->getLoc(),
                          "Expecting concrete bit init in `" + Op + "`");
        Segments.push_back({1, BI});
      } else if (const auto *SubDI = dyn_cast<DagInit>(Arg)) {
        buildRec(SubDI);
      } else {
        PrintFatalError(TheDef->getLoc(), "Unrecognized type of argument in `" +
                                              Op + "`: " + Arg->getAsString());
      }
    }
  } else if (Op == "operand") {
    // (operand <operand name>, <# of bits>,
    //          [(encoder <custom encoder>)][, (decoder <custom decoder>)])
    if (DI->getNumArgs() < 2)
      PrintFatalError(TheDef->getLoc(),
                      "Expecting at least 2 arguments for `operand`");
    HasDynamicSegment = true;
    const Init *OperandName = DI->getArg(0), *NumBits = DI->getArg(1);
    if (!isa<StringInit>(OperandName) || !isa<IntInit>(NumBits))
      PrintFatalError(TheDef->getLoc(), "Invalid argument types for `operand`");

    auto NumBitsVal = cast<IntInit>(NumBits)->getValue();
    if (NumBitsVal <= 0)
      PrintFatalError(TheDef->getLoc(), "Invalid number of bits for `operand`");

    auto [CustomEncoder, CustomDecoder] =
        getCustomCoders(DI->getArgs().slice(2));
    Segments.push_back({static_cast<unsigned>(NumBitsVal), OperandName,
                        CustomEncoder, CustomDecoder});
  } else if (Op == "slice") {
    // (slice <operand name>, <high / low bit>, <low / high bit>,
    //        [(encoder <custom encoder>)][, (decoder <custom decoder>)])
    if (DI->getNumArgs() < 3)
      PrintFatalError(TheDef->getLoc(),
                      "Expecting at least 3 arguments for `slice`");
    HasDynamicSegment = true;
    Init *OperandName = DI->getArg(0), *HiBit = DI->getArg(1),
         *LoBit = DI->getArg(2);
    if (!isa<StringInit>(OperandName) || !isa<IntInit>(HiBit) ||
        !isa<IntInit>(LoBit))
      PrintFatalError(TheDef->getLoc(), "Invalid argument types for `slice`");

    auto HiBitVal = cast<IntInit>(HiBit)->getValue(),
         LoBitVal = cast<IntInit>(LoBit)->getValue();
    if (HiBitVal < 0 || LoBitVal < 0)
      PrintFatalError(TheDef->getLoc(), "Invalid bit range for `slice`");
    bool NeedSwap = false;
    unsigned NumBits = 0U;
    if (HiBitVal < LoBitVal) {
      NeedSwap = true;
      NumBits = static_cast<unsigned>(LoBitVal - HiBitVal + 1);
    } else {
      NumBits = static_cast<unsigned>(HiBitVal - LoBitVal + 1);
    }

    auto [CustomEncoder, CustomDecoder] =
        getCustomCoders(DI->getArgs().slice(3));

    if (NeedSwap) {
      // Normalization: Hi bit should always be the second argument.
      Init *const NewArgs[] = {OperandName, LoBit, HiBit};
      Segments.push_back({NumBits,
                          DagInit::get(DI->getOperator(), nullptr, NewArgs, {}),
                          CustomEncoder, CustomDecoder});
    } else {
      Segments.push_back({NumBits, DI, CustomEncoder, CustomDecoder});
    }
  }
}

void VarLenCodeEmitterGen::run(raw_ostream &OS) {
  CodeGenTarget Target(Records);
  auto Insts = Records.getAllDerivedDefinitions("Instruction");

  auto NumberedInstructions = Target.getInstructionsByEnumValue();
  const CodeGenHwModes &HWM = Target.getHwModes();

  // The set of HwModes used by instruction encodings.
  std::set<unsigned> HwModes;
  for (const CodeGenInstruction *CGI : NumberedInstructions) {
    Record *R = CGI->TheDef;

    // Create the corresponding VarLenInst instance.
    if (R->getValueAsString("Namespace") == "TargetOpcode" ||
        R->getValueAsBit("isPseudo"))
      continue;

    if (const RecordVal *RV = R->getValue("EncodingInfos")) {
      if (auto *DI = dyn_cast_or_null<DefInit>(RV->getValue())) {
        EncodingInfoByHwMode EBM(DI->getDef(), HWM);
        for (auto &KV : EBM) {
          HwModes.insert(KV.first);
          Record *EncodingDef = KV.second;
          RecordVal *RV = EncodingDef->getValue("Inst");
          DagInit *DI = cast<DagInit>(RV->getValue());
          VarLenInsts.insert({EncodingDef, VarLenInst(DI, RV)});
        }
        continue;
      }
    }
    RecordVal *RV = R->getValue("Inst");
    DagInit *DI = cast<DagInit>(RV->getValue());
    VarLenInsts.insert({R, VarLenInst(DI, RV)});
  }

  // Emit function declaration
  OS << "void " << Target.getName()
     << "MCCodeEmitter::getBinaryCodeForInstr(const MCInst &MI,\n"
     << "    SmallVectorImpl<MCFixup> &Fixups,\n"
     << "    APInt &Inst,\n"
     << "    APInt &Scratch,\n"
     << "    const MCSubtargetInfo &STI) const {\n";

  // Emit instruction base values
  if (HwModes.empty()) {
    emitInstructionBaseValues(OS, NumberedInstructions, Target);
  } else {
    for (unsigned HwMode : HwModes)
      emitInstructionBaseValues(OS, NumberedInstructions, Target, (int)HwMode);
  }

  if (!HwModes.empty()) {
    OS << "  const unsigned **Index;\n";
    OS << "  const uint64_t *InstBits;\n";
    OS << "  unsigned HwMode = STI.getHwMode();\n";
    OS << "  switch (HwMode) {\n";
    OS << "  default: llvm_unreachable(\"Unknown hardware mode!\"); break;\n";
    for (unsigned I : HwModes) {
      OS << "  case " << I << ": InstBits = InstBits_" << HWM.getMode(I).Name
         << "; Index = Index_" << HWM.getMode(I).Name << "; break;\n";
    }
    OS << "  };\n";
  }

  // Emit helper function to retrieve base values.
  OS << "  auto getInstBits = [&](unsigned Opcode) -> APInt {\n"
     << "    unsigned NumBits = Index[Opcode][0];\n"
     << "    if (!NumBits)\n"
     << "      return APInt::getZeroWidth();\n"
     << "    unsigned Idx = Index[Opcode][1];\n"
     << "    ArrayRef<uint64_t> Data(&InstBits[Idx], "
     << "APInt::getNumWords(NumBits));\n"
     << "    return APInt(NumBits, Data);\n"
     << "  };\n";

  // Map to accumulate all the cases.
  std::map<std::string, std::vector<std::string>> CaseMap;

  // Construct all cases statement for each opcode
  for (Record *R : Insts) {
    if (R->getValueAsString("Namespace") == "TargetOpcode" ||
        R->getValueAsBit("isPseudo"))
      continue;
    std::string InstName =
        (R->getValueAsString("Namespace") + "::" + R->getName()).str();
    std::string Case = getInstructionCase(R, Target);

    CaseMap[Case].push_back(std::move(InstName));
  }

  // Emit initial function code
  OS << "  const unsigned opcode = MI.getOpcode();\n"
     << "  switch (opcode) {\n";

  // Emit each case statement
  for (const auto &C : CaseMap) {
    const std::string &Case = C.first;
    const auto &InstList = C.second;

    ListSeparator LS("\n");
    for (const auto &InstName : InstList)
      OS << LS << "    case " << InstName << ":";

    OS << " {\n";
    OS << Case;
    OS << "      break;\n"
       << "    }\n";
  }
  // Default case: unhandled opcode
  OS << "  default:\n"
     << "    std::string msg;\n"
     << "    raw_string_ostream Msg(msg);\n"
     << "    Msg << \"Not supported instr: \" << MI;\n"
     << "    report_fatal_error(Msg.str().c_str());\n"
     << "  }\n";
  OS << "}\n\n";
}

static void emitInstBits(raw_ostream &IS, raw_ostream &SS, const APInt &Bits,
                         unsigned &Index) {
  if (!Bits.getNumWords()) {
    IS.indent(4) << "{/*NumBits*/0, /*Index*/0},";
    return;
  }

  IS.indent(4) << "{/*NumBits*/" << Bits.getBitWidth() << ", "
               << "/*Index*/" << Index << "},";

  SS.indent(4);
  for (unsigned I = 0; I < Bits.getNumWords(); ++I, ++Index)
    SS << "UINT64_C(" << utostr(Bits.getRawData()[I]) << "),";
}

void VarLenCodeEmitterGen::emitInstructionBaseValues(
    raw_ostream &OS, ArrayRef<const CodeGenInstruction *> NumberedInstructions,
    CodeGenTarget &Target, int HwMode) {
  std::string IndexArray, StorageArray;
  raw_string_ostream IS(IndexArray), SS(StorageArray);

  const CodeGenHwModes &HWM = Target.getHwModes();
  if (HwMode == -1) {
    IS << "  static const unsigned Index[][2] = {\n";
    SS << "  static const uint64_t InstBits[] = {\n";
  } else {
    StringRef Name = HWM.getMode(HwMode).Name;
    IS << "  static const unsigned Index_" << Name << "[][2] = {\n";
    SS << "  static const uint64_t InstBits_" << Name << "[] = {\n";
  }

  unsigned NumFixedValueWords = 0U;
  for (const CodeGenInstruction *CGI : NumberedInstructions) {
    Record *R = CGI->TheDef;

    if (R->getValueAsString("Namespace") == "TargetOpcode" ||
        R->getValueAsBit("isPseudo")) {
      IS.indent(4) << "{/*NumBits*/0, /*Index*/0},\n";
      continue;
    }

    Record *EncodingDef = R;
    if (const RecordVal *RV = R->getValue("EncodingInfos")) {
      if (auto *DI = dyn_cast_or_null<DefInit>(RV->getValue())) {
        EncodingInfoByHwMode EBM(DI->getDef(), HWM);
        if (EBM.hasMode(HwMode))
          EncodingDef = EBM.get(HwMode);
      }
    }

    auto It = VarLenInsts.find(EncodingDef);
    if (It == VarLenInsts.end())
      PrintFatalError(EncodingDef, "VarLenInst not found for this record");
    const VarLenInst &VLI = It->second;

    unsigned i = 0U, BitWidth = VLI.size();

    // Start by filling in fixed values.
    APInt Value(BitWidth, 0);
    auto SI = VLI.begin(), SE = VLI.end();
    // Scan through all the segments that have fixed-bits values.
    while (i < BitWidth && SI != SE) {
      unsigned SegmentNumBits = SI->BitWidth;
      if (const auto *BI = dyn_cast<BitsInit>(SI->Value)) {
        for (unsigned Idx = 0U; Idx != SegmentNumBits; ++Idx) {
          auto *B = cast<BitInit>(BI->getBit(Idx));
          Value.setBitVal(i + Idx, B->getValue());
        }
      }
      if (const auto *BI = dyn_cast<BitInit>(SI->Value))
        Value.setBitVal(i, BI->getValue());

      i += SegmentNumBits;
      ++SI;
    }

    emitInstBits(IS, SS, Value, NumFixedValueWords);
    IS << '\t' << "// " << R->getName() << "\n";
    if (Value.getNumWords())
      SS << '\t' << "// " << R->getName() << "\n";
  }
  IS.indent(4) << "{/*NumBits*/0, /*Index*/0}\n  };\n";
  SS.indent(4) << "UINT64_C(0)\n  };\n";

  OS << IS.str() << SS.str();
}

std::string VarLenCodeEmitterGen::getInstructionCase(Record *R,
                                                     CodeGenTarget &Target) {
  std::string Case;
  if (const RecordVal *RV = R->getValue("EncodingInfos")) {
    if (auto *DI = dyn_cast_or_null<DefInit>(RV->getValue())) {
      const CodeGenHwModes &HWM = Target.getHwModes();
      EncodingInfoByHwMode EBM(DI->getDef(), HWM);
      Case += "      switch (HwMode) {\n";
      Case += "      default: llvm_unreachable(\"Unhandled HwMode\");\n";
      for (auto &KV : EBM) {
        Case += "      case " + itostr(KV.first) + ": {\n";
        Case += getInstructionCaseForEncoding(R, KV.second, Target);
        Case += "      break;\n";
        Case += "      }\n";
      }
      Case += "      }\n";
      return Case;
    }
  }
  return getInstructionCaseForEncoding(R, R, Target);
}

std::string VarLenCodeEmitterGen::getInstructionCaseForEncoding(
    Record *R, Record *EncodingDef, CodeGenTarget &Target) {
  auto It = VarLenInsts.find(EncodingDef);
  if (It == VarLenInsts.end())
    PrintFatalError(EncodingDef, "Parsed encoding record not found");
  const VarLenInst &VLI = It->second;
  size_t BitWidth = VLI.size();

  CodeGenInstruction &CGI = Target.getInstruction(R);

  std::string Case;
  raw_string_ostream SS(Case);
  // Resize the scratch buffer.
  if (BitWidth && !VLI.isFixedValueOnly())
    SS.indent(6) << "Scratch = Scratch.zext(" << BitWidth << ");\n";
  // Populate based value.
  SS.indent(6) << "Inst = getInstBits(opcode);\n";

  // Process each segment in VLI.
  size_t Offset = 0U;
  for (const auto &ES : VLI) {
    unsigned NumBits = ES.BitWidth;
    const Init *Val = ES.Value;
    // If it's a StringInit or DagInit, it's a reference to an operand
    // or part of an operand.
    if (isa<StringInit>(Val) || isa<DagInit>(Val)) {
      StringRef OperandName;
      unsigned LoBit = 0U;
      if (const auto *SV = dyn_cast<StringInit>(Val)) {
        OperandName = SV->getValue();
      } else {
        // Normalized: (slice <operand name>, <high bit>, <low bit>)
        const auto *DV = cast<DagInit>(Val);
        OperandName = cast<StringInit>(DV->getArg(0))->getValue();
        LoBit = static_cast<unsigned>(cast<IntInit>(DV->getArg(2))->getValue());
      }

      auto OpIdx = CGI.Operands.ParseOperandName(OperandName);
      unsigned FlatOpIdx = CGI.Operands.getFlattenedOperandNumber(OpIdx);
      StringRef CustomEncoder =
          CGI.Operands[OpIdx.first].EncoderMethodNames[OpIdx.second];
      if (ES.CustomEncoder.size())
        CustomEncoder = ES.CustomEncoder;

      SS.indent(6) << "Scratch.clearAllBits();\n";
      SS.indent(6) << "// op: " << OperandName.drop_front(1) << "\n";
      if (CustomEncoder.empty())
        SS.indent(6) << "getMachineOpValue(MI, MI.getOperand("
                     << utostr(FlatOpIdx) << ")";
      else
        SS.indent(6) << CustomEncoder << "(MI, /*OpIdx=*/" << utostr(FlatOpIdx);

      SS << ", /*Pos=*/" << utostr(Offset) << ", Scratch, Fixups, STI);\n";

      SS.indent(6) << "Inst.insertBits("
                   << "Scratch.extractBits(" << utostr(NumBits) << ", "
                   << utostr(LoBit) << ")"
                   << ", " << Offset << ");\n";
    }
    Offset += NumBits;
  }

  StringRef PostEmitter = R->getValueAsString("PostEncoderMethod");
  if (!PostEmitter.empty())
    SS.indent(6) << "Inst = " << PostEmitter << "(MI, Inst, STI);\n";

  return Case;
}

namespace llvm {

void emitVarLenCodeEmitter(RecordKeeper &R, raw_ostream &OS) {
  VarLenCodeEmitterGen(R).run(OS);
}

} // end namespace llvm
