#include <iostream>
#include <fstream>
#include <sstream>
#include "antlr4-runtime.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include <llvm/ExecutionEngine/Orc/Core.h>
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "parser/generated/BakeLexer.h"
#include "parser/generated/BakeParser.h"
#include "parser/TypeCheckVisitor.h"
#include "parser/CodeGenVisitor.h"

using namespace antlr4;
using namespace llvm;
using namespace llvm::orc;

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: ./bake <file.bake>" << std::endl;
    return 1;
  }

  std::ifstream file(argv[1]);

  if (!file) {
    std::cerr << "Error: Could not open file " << argv[1] << std::endl;
    return 1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string input = buffer.str();

  ANTLRInputStream stream(input);
  BakeLexer lexer(&stream);
  CommonTokenStream tokens(&lexer);
  BakeParser parser(&tokens);

  tree::ParseTree* tree = parser.program();
  TypeCheckVisitor* visitor = new TypeCheckVisitor();
  visitor->visit(tree);

  return 0;
  {
  CodeGenVisitor* visitor = new CodeGenVisitor();

  visitor->visit(tree);
  visitor->finish();


  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();

  std::unique_ptr<Module> module = visitor->getModule();
  std::unique_ptr<LLVMContext> context = visitor->getContext();

  // Create JIT
  auto jitExpected = LLJITBuilder().create();
  if (!jitExpected) {
    errs() << "Failed to create JIT\n";
    return 1;
  }
  auto jit = std::move(*jitExpected);

  // Add host process symbols (so printf works)
  jit->getMainJITDylib().addGenerator(
      cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(
          jit->getDataLayout().getGlobalPrefix())));

  // Wrap module in ThreadSafeModule
  ThreadSafeModule tsm(std::move(module), std::move(context));

  if (auto err = jit->addIRModule(std::move(tsm))) {
    errs() << "Failed to add module\n";
    return 1;
  }

  auto addrExpected = jit->lookup("main");
  if (!addrExpected) {
    errs() << "Failed to lookup main\n";
    return 1;
  }

  // This is an ExecutorAddr in LLVM 19
  ExecutorAddr mainAddr = *addrExpected;

  // Turn into a callable function pointer
  auto *mainPtr = mainAddr.toPtr<int (*)()>();

  return mainPtr();
  }
}
