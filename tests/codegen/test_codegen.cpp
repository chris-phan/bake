#include "parser/TypeCheckVisitor.h"
#include "parser/CodeGenVisitor.h"
#include "parser/generated/BakeLexer.h"
#include "parser/generated/BakeParser.h"
#include "gtest/gtest.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include <llvm/ExecutionEngine/Orc/Core.h>
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace antlr4;

namespace fs = std::filesystem;
using namespace llvm;
using namespace llvm::orc;

void assertTypecheck(std::string input) {
  ANTLRInputStream stream(input);
  BakeLexer lexer(&stream);
  CommonTokenStream tokens(&lexer);
  BakeParser parser(&tokens);

  tree::ParseTree *tree = parser.program();
  TypeCheckVisitor *visitor = new TypeCheckVisitor();

  testing::internal::CaptureStderr();

  visitor->visit(tree);

  std::string output = testing::internal::GetCapturedStderr();

  // Can only codegen if there are 0 errors when typechecking
  ASSERT_EQ(output.length(), 0);
}

void cgen(std::string input) {
  ANTLRInputStream stream(input);
  BakeLexer lexer(&stream);
  CommonTokenStream tokens(&lexer);
  BakeParser parser(&tokens);

  tree::ParseTree *tree = parser.program();
  CodeGenVisitor *visitor = new CodeGenVisitor();

  testing::internal::CaptureStderr();

  visitor->visit(tree);
  visitor->finish();

  std::string output= testing::internal::GetCapturedStderr();

  // Should have no errors during codegen
  EXPECT_EQ(output.length(), 0);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();

  std::unique_ptr<Module> module = visitor->getModule();
  std::unique_ptr<LLVMContext> context = visitor->getContext();

  // Create JIT
  auto jitExpected = LLJITBuilder().create();
  if (!jitExpected) {
    errs() << "Failed to create JIT\n";
    return;
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
    return;
  }

  auto addrExpected = jit->lookup("main");
  if (!addrExpected) {
    errs() << "Failed to lookup main\n";
    return;
  }

  // This is an ExecutorAddr in LLVM 19
  ExecutorAddr mainAddr = *addrExpected;

  // Turn into a callable function pointer
  auto *mainPtr = mainAddr.toPtr<int (*)()>();

  // Program should return 0 (success)
  EXPECT_EQ(mainPtr(), 0);
}

void runTest(std::string pathSegment) {
  fs::path p = fs::path(__FILE__).parent_path() / pathSegment;
  std::ifstream file(p);

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string input = buffer.str();

  assertTypecheck(input);
  cgen(input);
}

TEST(Codegen, Concat) {
  testing::internal::CaptureStdout();
  runTest("input/concat.bake");
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_EQ(output.compare("helloworld\n\nbyeworld\n"), 0);
}
