#include <circt/Dialect/Comb/CombDialect.h>
#include <circt/Dialect/FIRRTL/FIRRTLDialect.h>
#include <circt/Dialect/SV/SVDialect.h>
#include <circt/Dialect/Seq/SeqDialect.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/Tools/mlir-lsp-server/MlirLspServerMain.h>

using namespace circt;

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<firrtl::FIRRTLDialect, sv::SVDialect, seq::SeqDialect,
                  hw::HWDialect, comb::CombDialect>();
  return mlir::failed(mlir::MlirLspServerMain(argc, argv, registry));
}
