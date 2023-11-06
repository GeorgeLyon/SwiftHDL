//===- Transforms/Instrumentation/InstrProfiling.h --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file provides the interface for LLVM's PGO Instrumentation lowering
/// pass.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_INSTRPROFILING_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_INSTRPROFILING_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PassManager.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Transforms/Instrumentation.h"
#include <cstdint>
#include <cstring>
#include <vector>

namespace llvm {

class TargetLibraryInfo;
using LoadStorePair = std::pair<Instruction *, Instruction *>;

/// Instrumentation based profiling lowering pass. This pass lowers
/// the profile instrumented code generated by FE or the IR based
/// instrumentation pass.
class InstrProfiling : public PassInfoMixin<InstrProfiling> {
public:
  InstrProfiling() : IsCS(false) {}
  InstrProfiling(const InstrProfOptions &Options, bool IsCS = false)
      : Options(Options), IsCS(IsCS) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  bool run(Module &M,
           std::function<const TargetLibraryInfo &(Function &F)> GetTLI);

private:
  InstrProfOptions Options;
  Module *M;
  Triple TT;
  std::function<const TargetLibraryInfo &(Function &F)> GetTLI;
  struct PerFunctionProfileData {
    uint32_t NumValueSites[IPVK_Last + 1];
    GlobalVariable *RegionCounters = nullptr;
    GlobalVariable *DataVar = nullptr;
    GlobalVariable *RegionBitmaps = nullptr;

    PerFunctionProfileData() {
      memset(NumValueSites, 0, sizeof(uint32_t) * (IPVK_Last + 1));
    }
  };
  DenseMap<GlobalVariable *, PerFunctionProfileData> ProfileDataMap;
  /// If runtime relocation is enabled, this maps functions to the load
  /// instruction that produces the profile relocation bias.
  DenseMap<const Function *, LoadInst *> FunctionToProfileBiasMap;
  std::vector<GlobalValue *> CompilerUsedVars;
  std::vector<GlobalValue *> UsedVars;
  std::vector<GlobalVariable *> ReferencedNames;
  GlobalVariable *NamesVar;
  size_t NamesSize;

  // Is this lowering for the context-sensitive instrumentation.
  bool IsCS;

  // vector of counter load/store pairs to be register promoted.
  std::vector<LoadStorePair> PromotionCandidates;

  int64_t TotalCountersPromoted = 0;

  /// Lower instrumentation intrinsics in the function. Returns true if there
  /// any lowering.
  bool lowerIntrinsics(Function *F);

  /// Register-promote counter loads and stores in loops.
  void promoteCounterLoadStores(Function *F);

  /// Returns true if relocating counters at runtime is enabled.
  bool isRuntimeCounterRelocationEnabled() const;

  /// Returns true if profile counter update register promotion is enabled.
  bool isCounterPromotionEnabled() const;

  /// Count the number of instrumented value sites for the function.
  void computeNumValueSiteCounts(InstrProfValueProfileInst *Ins);

  /// Replace instrprof.value.profile with a call to runtime library.
  void lowerValueProfileInst(InstrProfValueProfileInst *Ins);

  /// Replace instrprof.cover with a store instruction to the coverage byte.
  void lowerCover(InstrProfCoverInst *Inc);

  /// Replace instrprof.timestamp with a call to
  /// INSTR_PROF_PROFILE_SET_TIMESTAMP.
  void lowerTimestamp(InstrProfTimestampInst *TimestampInstruction);

  /// Replace instrprof.increment with an increment of the appropriate value.
  void lowerIncrement(InstrProfIncrementInst *Inc);

  /// Force emitting of name vars for unused functions.
  void lowerCoverageData(GlobalVariable *CoverageNamesVar);

  /// Replace instrprof.mcdc.tvbitmask.update with a shift and or instruction
  /// using the index represented by the a temp value into a bitmap.
  void lowerMCDCTestVectorBitmapUpdate(InstrProfMCDCTVBitmapUpdate *Ins);

  /// Replace instrprof.mcdc.temp.update with a shift and or instruction using
  /// the corresponding condition ID.
  void lowerMCDCCondBitmapUpdate(InstrProfMCDCCondBitmapUpdate *Ins);

  /// Compute the address of the counter value that this profiling instruction
  /// acts on.
  Value *getCounterAddress(InstrProfCntrInstBase *I);

  /// Get the region counters for an increment, creating them if necessary.
  ///
  /// If the counter array doesn't yet exist, the profile data variables
  /// referring to them will also be created.
  GlobalVariable *getOrCreateRegionCounters(InstrProfCntrInstBase *Inc);

  /// Create the region counters.
  GlobalVariable *createRegionCounters(InstrProfCntrInstBase *Inc,
                                       StringRef Name,
                                       GlobalValue::LinkageTypes Linkage);

  /// Compute the address of the test vector bitmap that this profiling
  /// instruction acts on.
  Value *getBitmapAddress(InstrProfMCDCTVBitmapUpdate *I);

  /// Get the region bitmaps for an increment, creating them if necessary.
  ///
  /// If the bitmap array doesn't yet exist, the profile data variables
  /// referring to them will also be created.
  GlobalVariable *getOrCreateRegionBitmaps(InstrProfMCDCBitmapInstBase *Inc);

  /// Create the MC/DC bitmap as a byte-aligned array of bytes associated with
  /// an MC/DC Decision region. The number of bytes required is indicated by
  /// the intrinsic used (type InstrProfMCDCBitmapInstBase).  This is called
  /// as part of setupProfileSection() and is conceptually very similar to
  /// what is done for profile data counters in createRegionCounters().
  GlobalVariable *createRegionBitmaps(InstrProfMCDCBitmapInstBase *Inc,
                                      StringRef Name,
                                      GlobalValue::LinkageTypes Linkage);

  /// Set Comdat property of GV, if required.
  void maybeSetComdat(GlobalVariable *GV, Function *Fn, StringRef VarName);

  /// Setup the sections into which counters and bitmaps are allocated.
  GlobalVariable *setupProfileSection(InstrProfInstBase *Inc,
                                      InstrProfSectKind IPSK);

  /// Create INSTR_PROF_DATA variable for counters and bitmaps.
  void createDataVariable(InstrProfCntrInstBase *Inc,
                          InstrProfMCDCBitmapParameters *Update);

  /// Emit the section with compressed function names.
  void emitNameData();

  /// Emit value nodes section for value profiling.
  void emitVNodes();

  /// Emit runtime registration functions for each profile data variable.
  void emitRegistration();

  /// Emit the necessary plumbing to pull in the runtime initialization.
  /// Returns true if a change was made.
  bool emitRuntimeHook();

  /// Add uses of our data variables and runtime hook.
  void emitUses();

  /// Create a static initializer for our data, on platforms that need it,
  /// and for any profile output file that was specified.
  void emitInitialization();
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_INSTRUMENTATION_INSTRPROFILING_H
