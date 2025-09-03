#include "parser/TypeCheckVisitor.h"
#include "parser/generated/BakeLexer.h"
#include "parser/generated/BakeParser.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace antlr4;

namespace fs = std::filesystem;

std::string join(std::vector<std::string> strings, std::string delim) {
	if (strings.size() == 0) {
		return "";
	}

	std::string res = strings[0];
	for (size_t i = 1; i < strings.size(); ++i) {
		res += delim + strings[i];
	}

	return res;
}

std::string typecheck(std::string pathSegment) {
  fs::path p = fs::path(__FILE__).parent_path() / pathSegment;
  std::ifstream file(p);

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string input = buffer.str();

  ANTLRInputStream stream(input);
  BakeLexer lexer(&stream);
  CommonTokenStream tokens(&lexer);
  BakeParser parser(&tokens);

  tree::ParseTree *tree = parser.program();
  TypeCheckVisitor *visitor = new TypeCheckVisitor();

	testing::internal::CaptureStderr();

	visitor->visit(tree);

	return testing::internal::GetCapturedStderr();
}

TEST(TypeCheck, Add) {
	std::string output = typecheck("input/add.bake");

  std::vector<std::string> errs = {
		"Error: Line 23: Type mismatch: Can't add Int to String\n",
		"Error: Line 24: Type mismatch: Can't add String to Int\n",
		"Error: Line 26: Type mismatch: Can't assign Int to String\n",
		"Error: Line 27: Type mismatch: Can't assign Int to String\n",
		"Error: Line 28: Type mismatch: Can't assign Int to String\n",
		"Error: Line 30: Type mismatch: Can't assign String to Int\n",
		"Error: Line 31: Type mismatch: Can't assign String to Int\n",
		"Error: Line 32: Type mismatch: Can't assign String to Int\n",
		"Error: Line 34: Type mismatch: Can't add different types, Int and String\n",
		"Error: Line 35: Type mismatch: Can't add different types, String and Int\n",
		"Error: Line 36: Type mismatch: Can't add different types, Int and String\n",
		"Error: Line 37: Type mismatch: Can't add different types, String and Int\n"
	};

	EXPECT_EQ(output, join(errs, ""));
}

TEST(TypeCheck, Subtract) {
	std::string output = typecheck("input/subtract.bake");

  std::vector<std::string> errs = {
		"Error: Line 17: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 18: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 19: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 19: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 20: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 20: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 22: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 23: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 25: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 26: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 27: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 29: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 30: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 31: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 33: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 34: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 35: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 35: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 36: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 36: Type mismatch: expected StrA to be type Int but is String instead\n",
	};

	EXPECT_EQ(output, join(errs, ""));
}

TEST(TypeCheck, Multiply) {
	std::string output = typecheck("input/multiply.bake");

  std::vector<std::string> errs = {
		"Error: Line 17: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 18: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 19: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 19: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 20: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 20: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 22: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 23: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 25: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 26: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 27: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 29: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 30: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 31: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 33: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 34: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 35: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 35: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 36: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 36: Type mismatch: expected StrA to be type Int but is String instead\n",
	};

	EXPECT_EQ(output, join(errs, ""));
}

TEST(TypeCheck, Divide) {
	std::string output = typecheck("input/divide.bake");

  std::vector<std::string> errs = {
		"Error: Line 17: divide by 0 error\n",
		"Error: Line 18: divide by 0 error\n",
		"Error: Line 19: divide by 0 error\n",
		"Error: Line 20: divide by 0 error\n",
		"Error: Line 22: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 23: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 24: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 24: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 25: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 25: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 27: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 28: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 30: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 31: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 32: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 34: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 35: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 36: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 38: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 39: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 40: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 40: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 41: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 41: Type mismatch: expected StrA to be type Int but is String instead\n",
	};

	EXPECT_EQ(output, join(errs, ""));
}

TEST(TypeCheck, Modulo) {
	std::string output = typecheck("input/modulo.bake");

  std::vector<std::string> errs = {
		"Error: Line 18: modulo by 0 error\n",
		"Error: Line 19: modulo by 0 error\n",
		"Error: Line 21: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 21: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 22: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 22: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 23: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 23: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 25: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 26: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 27: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 27: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 28: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 28: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 30: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 31: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 32: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 34: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 35: Type mismatch: expected \"a\" to be type Int but is String instead\n",
		"Error: Line 36: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 38: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 39: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 40: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 40: Type mismatch: expected StrA to be type Int but is String instead\n",
		"Error: Line 41: Type mismatch: expected StrB to be type Int but is String instead\n",
		"Error: Line 41: Type mismatch: expected StrA to be type Int but is String instead\n",
	};

	EXPECT_EQ(output, join(errs, ""));
}

TEST(TypeCheck, If) {
	std::string output = typecheck("input/if.bake");

  std::vector<std::string> errs = {
		"Error: Line 42: '<' is unsupported for type String\n",
		"Error: Line 43: '<=' is unsupported for type String\n",
		"Error: Line 44: '>' is unsupported for type String\n",
		"Error: Line 45: '>=' is unsupported for type String\n",
		"Error: Line 47: '<' is unsupported for type String\n",
		"Error: Line 48: '<=' is unsupported for type String\n",
		"Error: Line 49: '>' is unsupported for type String\n",
		"Error: Line 50: '>=' is unsupported for type String\n",
		"Error: Line 52: '<' is unsupported for type String\n",
		"Error: Line 53: '<=' is unsupported for type String\n",
		"Error: Line 54: '>' is unsupported for type String\n",
		"Error: Line 55: '>=' is unsupported for type String\n",
	};

	EXPECT_EQ(output, join(errs, ""));
}

TEST(TypeCheck, Loop) {
	std::string output = typecheck("input/loop.bake");

  std::vector<std::string> errs = {
		"Error: Line 91: '<' is unsupported for type String\n",
		"Error: Line 94: '<=' is unsupported for type String\n",
		"Error: Line 97: '>' is unsupported for type String\n",
		"Error: Line 100: '>=' is unsupported for type String\n",
		"Error: Line 104: '<' is unsupported for type String\n",
		"Error: Line 107: '<=' is unsupported for type String\n",
		"Error: Line 110: '>' is unsupported for type String\n",
		"Error: Line 113: '>=' is unsupported for type String\n",
		"Error: Line 117: '<' is unsupported for type String\n",
		"Error: Line 120: '<=' is unsupported for type String\n",
		"Error: Line 123: '>' is unsupported for type String\n",
		"Error: Line 126: '>=' is unsupported for type String\n",
	};

	EXPECT_EQ(output, join(errs, ""));
}
