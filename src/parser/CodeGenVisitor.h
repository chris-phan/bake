#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "generated/BakeBaseVisitor.h"
#include "generated/BakeLexer.h"
#include "generated/BakeParser.h"
#include <iostream>

using namespace llvm;

class CodeGenVisitor : public BakeBaseVisitor {
private:
  enum class Kind {
      Int,
      String,
      ArrayInt,
      ArrayString
  };

  std::unique_ptr<LLVMContext> theContext;
  std::unique_ptr<IRBuilder<>> builder;
  std::unique_ptr<Module> theModule;
  std::map<std::string, Value *> namedValues;
  std::map<std::string, AllocaInst*> variables;

public:
  CodeGenVisitor() {
    theContext = std::make_unique<LLVMContext>();
    theModule = std::make_unique<Module>("bake compiler", *theContext);
    builder = std::make_unique<IRBuilder<>>(*theContext);

    FunctionType *mainType = FunctionType::get(builder->getInt32Ty(), false);
    Function *mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", theModule.get());
    
    BasicBlock *entry = BasicBlock::Create(*theContext, "entry", mainFunc);
    builder->SetInsertPoint(entry);
  }

  virtual std::any visitProgram(BakeParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitName(BakeParser::NameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIngredients(BakeParser::IngredientsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIngredientsHeading(BakeParser::IngredientsHeadingContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEquipment(BakeParser::EquipmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEquipmentHeading(BakeParser::EquipmentHeadingContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitItem(BakeParser::ItemContext *ctx) override {
      return visitChildren(ctx);
  }

  virtual std::any visitItemDecl(BakeParser::ItemDeclContext *ctx) override {
    std::string varName = ctx->ID()->getText();
    Value *val = std::any_cast<Value*>(visit(ctx->type()));
    namedValues[varName] = val;

    /*Kind varType = typeAndVal.first;*/
    /*Value *initVal = typeAndVal.second;*/
    /**/
    /*AllocaInst *alloca;*/
    /*switch (varType) {*/
    /*  case Kind::Int: {*/
    /*    alloca = builder->CreateAlloca(builder->getInt32Ty(), nullptr, varName);*/
    /*    builder->CreateStore(initVal, alloca);*/
    /*    break;*/
    /*  }*/
    /*  case Kind::String: {*/
    /*    int strLen = ctx->type()->stringType()->StringLiteral()->getText().length();*/
    /*    strLen -= 2;  // remove quotes ("") from the count*/
    /*    std::cout << "(string) strLen: " << strLen << std::endl;*/
    /**/
    /*    Type *charType = builder->getInt8Ty();*/
    /*    Type *arrType = ArrayType::get(charType, strLen);*/
    /*    alloca = builder->CreateAlloca(arrType, nullptr, varName);*/
    /*    break;*/
    /*  }*/
    /*  case Kind::ArrayInt: {*/
    /*    int arrLen = std::stoi(ctx->type()->arrayType()->children[0]->getText());*/
    /*    std::cout << "(array int) arrLen: " << arrLen << std::endl;*/
    /**/
    /*    Type *intType = builder->getInt32Ty();*/
    /*    Type *arrType = ArrayType::get(intType, arrLen);*/
    /*    alloca = builder->CreateAlloca(arrType, nullptr, varName);*/
    /*    break;*/
    /*  }*/
    /*  case Kind::ArrayString: {*/
    /*    int arrLen = std::stoi(ctx->type()->arrayType()->children[0]->getText());*/
    /*    std::cout << "(array string) arrLen: " << arrLen << std::endl;*/
    /**/
    /*    Type *strPtrType = builder->getPtrTy();*/
    /*    Type *arrType = ArrayType::get(strPtrType, arrLen);*/
    /*    alloca = builder->CreateAlloca(arrType, nullptr, varName);*/
    /*    break;*/
    /*  }*/
    /*  default:*/
    /*    break;*/
    /*}*/

    return nullptr;
  }

  virtual std::any visitType(BakeParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIntType(BakeParser::IntTypeContext *ctx) override {
    int intVal = std::stoi(ctx->Int()->getText());
    Value *globalInt = new GlobalVariable(
      *theModule,
      builder->getInt32Ty(),
      false,
      GlobalValue::PrivateLinkage,
      builder->getInt32(intVal),
      "globalInt"
    );

    return globalInt;
  }

  virtual std::any visitStringType(BakeParser::StringTypeContext *ctx) override {
    std::string strVal = ctx->StringLiteral()->getText();

    // Trim starting and end quotes ("")
    strVal = strVal.substr(1, strVal.length() - 2);

    auto strConst = ConstantDataArray::getString(*theContext, strVal, true);
    GlobalVariable *globalStr = new GlobalVariable(
      *theModule,
      strConst->getType(),
      true,
      GlobalValue::PrivateLinkage,
      strConst,
      "string"
    );

    // Get a pointer to the first char
    Value *zero = builder->getInt32(0);
    Value *idxs[] = {zero, zero};
    return builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, idxs, "strPtr");
  }

  virtual std::any visitArrayInt(BakeParser::ArrayIntContext *ctx) override {
    int arrLen = std::stoi(ctx->Int(0)->getText());
    int initVal = std::stoi(ctx->Int(1)->getText());
    std::cout << "visitArrayInt: arrLen: " << arrLen << " initVal: " << initVal << std::endl;
    std::vector<Constant*> initVals(arrLen, ConstantInt::get(builder->getInt32Ty(), initVal));

    ArrayType *arrType = ArrayType::get(builder->getInt32Ty(), arrLen);
    GlobalVariable *globalIntArr = new GlobalVariable(
      *theModule,
      arrType,
      false,
      GlobalValue::PrivateLinkage,
      ConstantArray::get(arrType, initVals),
      "intArr"
    );

    // Get a pointer to the first int
    Value *zero = builder->getInt32(0);
    Value *idxs[] = {zero, zero};
    return builder->CreateInBoundsGEP(globalIntArr->getValueType(), globalIntArr, idxs, "intArrPtr");
  }

  virtual std::any visitArrayString(BakeParser::ArrayStringContext *ctx) override {
    int arrLen = std::stoi(ctx->Int()->getText());
    std::string initVal = ctx->StringLiteral()->getText();

    // Trim starting and end quotes ("")
    initVal = initVal.substr(1, initVal.length() - 2);

    std::cout << "visitArrayString: arrLen: " << arrLen << " initVal: " << initVal << std::endl;

    // Create pointers for each string in the array
    std::vector<Constant*> strPtrs;
    Value *zero = builder->getInt32(0);
    for (int i = 0; i < arrLen; ++i) {
      auto strConst = ConstantDataArray::getString(*theContext, initVal, true);
      GlobalVariable *globalStr = new GlobalVariable(
        *theModule,
        strConst->getType(),
        true,
        GlobalValue::PrivateLinkage,
        strConst,
        "string"
      );

      auto strPtr = ConstantExpr::getInBoundsGetElementPtr(globalStr->getValueType(), globalStr, {zero, zero});
      strPtrs.push_back(strPtr);
    }

    ArrayType *arrType = ArrayType::get(PointerType::get(Type::getInt8Ty(*theContext), 0), arrLen);
    Constant *strArrConst = ConstantArray::get(arrType, strPtrs);
    GlobalVariable *globalStrArr = new GlobalVariable(
      *theModule,
      arrType,
      false,
      GlobalValue::PrivateLinkage,
      strArrConst,
      "strArr"
    );

    theModule->print(outs(), nullptr);
    return builder->CreateInBoundsGEP(globalStrArr->getValueType(), globalStrArr, {zero, zero}, "strArrPtr");
  }

  /*virtual std::any visitInstructions(BakeParser::InstructionsContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitInstructionsHeading(BakeParser::InstructionsHeadingContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitInstOpStmt(BakeParser::InstOpStmtContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitInstLoopStmt(BakeParser::InstLoopStmtContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitInstIfStmt(BakeParser::InstIfStmtContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitOpAdd(BakeParser::OpAddContext *ctx) override {*/
  /*  Kind operand1 = std::any_cast<Kind>(visit(ctx->value(0)));*/
  /*  if (ctx->And()) {*/
  /*    Kind operand2 = std::any_cast<Kind>(visit(ctx->value(1)));*/
  /*    Kind destination = std::any_cast<Kind>(visit(ctx->value(2)));*/
  /**/
  /*    if (operand1 != operand2) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: Can't add different types, "*/
  /*                << getKindName(operand1)*/
  /*                << " and "*/
  /*                << getKindName(operand2)*/
  /*                << std::endl;*/
  /*    } else if (operand1 != destination) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: Can't assign "*/
  /*                << getKindName(operand1)*/
  /*                << " to "*/
  /*                << getKindName(destination)*/
  /*                << std::endl;*/
  /*    }*/
  /*  } else {*/
  /*    Kind destination = std::any_cast<Kind>(visit(ctx->value(1)));*/
  /*    if (operand1 != destination) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: Can't add "*/
  /*                << getKindName(operand1)*/
  /*                << " to "*/
  /*                << getKindName(destination)*/
  /*                << std::endl;*/
  /*    }*/
  /*  }*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitOpSub(BakeParser::OpSubContext *ctx) override {*/
  /*  Kind toRemoveType = std::any_cast<Kind>(visit(ctx->value(0)));*/
  /*  Kind removeFromType = std::any_cast<Kind>(visit(ctx->value(1)));*/
  /*  if (ctx->preposition()) {*/
  /*    // Specified destination*/
  /*    std::string destName = ctx->ID()->getText();*/
  /*    assertVariableExists(ctx, destName);*/
  /**/
  /*    Kind destinationType = variableTypes[destName];*/
  /*    if (destinationType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->ID()->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(destinationType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*    }*/
  /*  }*/
  /*  if (toRemoveType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->value(0)->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(toRemoveType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*  } else if (removeFromType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->value(1)->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(removeFromType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*  }*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitOpMult(BakeParser::OpMultContext *ctx) override {*/
  /*  Kind toMultiplyType = std::any_cast<Kind>(visit(ctx->value(0)));*/
  /*  Kind multiplierType = std::any_cast<Kind>(visit(ctx->value(1)));*/
  /*  if (ctx->preposition()) {*/
  /*    // Specified destination*/
  /*    std::string destName = ctx->ID()->getText();*/
  /*    assertVariableExists(ctx, destName);*/
  /**/
  /*    Kind destinationType = variableTypes[destName];*/
  /*    if (destinationType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->ID()->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(destinationType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*    }*/
  /*  }*/
  /*  if (toMultiplyType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->value(0)->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(toMultiplyType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*  } else if (multiplierType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->value(1)->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(multiplierType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*  }*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitOpDiv(BakeParser::OpDivContext *ctx) override {*/
  /*  Kind dividendType = std::any_cast<Kind>(visit(ctx->value(0)));*/
  /*  Kind divisorType = std::any_cast<Kind>(visit(ctx->value(1)));*/
  /*  if (ctx->preposition()) {*/
  /*    // Specified destination*/
  /*    std::string destName = ctx->ID()->getText();*/
  /*    assertVariableExists(ctx, destName);*/
  /**/
  /*    Kind destinationType = variableTypes[destName];*/
  /*    if (destinationType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->ID()->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(destinationType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*    }*/
  /*  }*/
  /*  if (dividendType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->value(0)->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(dividendType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*  } else if (divisorType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->value(1)->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(divisorType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*  }*/
  /**/
  /*  if (divisorType == Kind::Int && ctx->value(1)->getText() == "0") {*/
  /*    std::cerr << "Error: Line "*/
  /*              << ctx->getStart()->getLine()*/
  /*              << ": divide by 0 error"*/
  /*              << std::endl;*/
  /*  }*/
  /**/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitOpMod(BakeParser::OpModContext *ctx) override {*/
  /*  Kind baseValueType = std::any_cast<Kind>(visit(ctx->value(0)));*/
  /*  Kind modByType = std::any_cast<Kind>(visit(ctx->value(1)));*/
  /**/
  /*  std::string destName = ctx->ID()->getText();*/
  /*  assertVariableExists(ctx, destName);*/
  /**/
  /*  Kind destinationType = variableTypes[destName];*/
  /*  if (destinationType != Kind::Int) {*/
  /*    std::cerr << "Error: Line "*/
  /*              << ctx->getStart()->getLine()*/
  /*              << ": Type mismatch: expected "*/
  /*              << ctx->ID()->getText()*/
  /*              << " to be type Int but is "*/
  /*              << getKindName(destinationType)*/
  /*              << " instead"*/
  /*              << std::endl;*/
  /*  }*/
  /**/
  /*  if (baseValueType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->value(0)->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(baseValueType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*  } else if (modByType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": Type mismatch: expected "*/
  /*                << ctx->value(1)->getText()*/
  /*                << " to be type Int but is "*/
  /*                << getKindName(modByType)*/
  /*                << " instead"*/
  /*                << std::endl;*/
  /*  }*/
  /**/
  /*  if (modByType == Kind::Int && ctx->value(1)->getText() == "0") {*/
  /*    std::cerr << "Error: Line "*/
  /*              << ctx->getStart()->getLine()*/
  /*              << ": modulo by 0 error"*/
  /*              << std::endl;*/
  /*  }*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitOpServe(BakeParser::OpServeContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitIfStmt(BakeParser::IfStmtContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitBinaryExprCond(BakeParser::BinaryExprCondContext *ctx) override {*/
  /*  // Enforce <, <=, >, and >= to use Ints*/
  /*  auto condType = ctx->conditionOp()->getStart()->getType();*/
  /*  std::vector<int> intComparisons = { BakeLexer::LessThan, BakeLexer::LessThanEq, BakeLexer::GreaterThan, BakeLexer::GreaterThanEq };*/
  /**/
  /*  if (std::find(intComparisons.begin(), intComparisons.end(), condType) != intComparisons.end()) {*/
  /*    Kind lhsType = std::any_cast<Kind>(visit(ctx->value(0)));*/
  /*    Kind rhsType = std::any_cast<Kind>(visit(ctx->value(1)));*/
  /*    if (lhsType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": '"*/
  /*                << ctx->conditionOp()->getText()*/
  /*                << "' is unsupported for type "*/
  /*                << getKindName(lhsType)*/
  /*                << std::endl;*/
  /*    } else if (rhsType != Kind::Int) {*/
  /*      std::cerr << "Error: Line "*/
  /*                << ctx->getStart()->getLine()*/
  /*                << ": '"*/
  /*                << ctx->conditionOp()->getText()*/
  /*                << "' is unsupported for type "*/
  /*                << getKindName(rhsType)*/
  /*                << std::endl;*/
  /*    }*/
  /*  }*/
  /**/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitBinaryExprOr(BakeParser::BinaryExprOrContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitBinaryExprAnd(BakeParser::BinaryExprAndContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitLoopStmt(BakeParser::LoopStmtContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitValueID(BakeParser::ValueIDContext *ctx) override {*/
  /*  std::string varName = ctx->ID()->getText();*/
  /*  assertVariableExists(ctx, varName);*/
  /*  return variableTypes[varName];*/
  /*}*/
  /**/
  /*// Arrays can only be accessed using integers or variables that are integers*/
  /*// Strings can't be indexed*/
  /*virtual std::any visitValueArrayAccessID(BakeParser::ValueArrayAccessIDContext *ctx) override {*/
  /*  // Ensure that the variables used in the array access exist*/
  /*  std::string arrayName = ctx->ID(0)->getText();*/
  /*  std::string accessIDName = ctx->ID(0)->getText();*/
  /*  assertVariableExists(ctx, arrayName);*/
  /*  assertVariableExists(ctx, accessIDName);*/
  /**/
  /*  // Ensure that the variable being indexed is an actual array type*/
  /*  Kind arrayType = variableTypes[arrayName];*/
  /*  if (arrayType != Kind::ArrayInt || arrayType != Kind::ArrayString) {*/
  /*    std::cerr << "Error: Can't index " << arrayName << " which is of type " << getKindName(arrayType) << std::endl;*/
  /*    exit(1);*/
  /*  }*/
  /**/
  /*  // Ensure that the ID used as an index is an Int*/
  /*  Kind accessIDType = variableTypes[accessIDName];*/
  /*  if (accessIDType != Kind::Int) {*/
  /*    std::cerr << "Error: " << accessIDName << " is of type " << getKindName(accessIDType) << " but has to be of type Int" << std::endl;*/
  /*    exit(1);*/
  /*  }*/
  /**/
  /*  // Expression resolves to a single Int or a single String*/
  /*  if (arrayType == Kind::ArrayInt) {*/
  /*    return Kind::Int;*/
  /*  }*/
  /*  return Kind::String;*/
  /*}*/
  /**/
  /*virtual std::any visitValueArrayAccessInt(BakeParser::ValueArrayAccessIntContext *ctx) override {*/
  /*  // Ensure that the variable being indexed exists*/
  /*  std::string arrayName = ctx->ID()->getText();*/
  /*  assertVariableExists(ctx, arrayName);*/
  /**/
  /*  // Ensure that the variable being indexed is an actual array type*/
  /*  Kind arrayType = variableTypes[arrayName];*/
  /*  if (arrayType != Kind::ArrayInt || arrayType != Kind::ArrayString) {*/
  /*    std::cerr << "Error: Can't index " << arrayName << " which is of type " << getKindName(arrayType) << std::endl;*/
  /*    exit(1);*/
  /*  }*/
  /**/
  /*  // Expression resolves to a single Int or a single String*/
  /*  if (arrayType == Kind::ArrayInt) {*/
  /*    return Kind::Int;*/
  /*  }*/
  /*  return Kind::String;*/
  /*}*/
  /**/
  /*virtual std::any visitValueInt(BakeParser::ValueIntContext *ctx) override {*/
  /*  return Kind::Int;*/
  /*}*/
  /**/
  /*virtual std::any visitValueString(BakeParser::ValueStringContext *ctx) override {*/
  /*  return Kind::String;*/
  /*}*/
  /**/
  /*virtual std::any visitCondOpIs(BakeParser::CondOpIsContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitCondOpIsNot(BakeParser::CondOpIsNotContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitCondOpLessThan(BakeParser::CondOpLessThanContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitCondOpGreaterThan(BakeParser::CondOpGreaterThanContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitCondOpLessThanEq(BakeParser::CondOpLessThanEqContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitCondOpGreaterThanEq(BakeParser::CondOpGreaterThanEqContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitStatementEndMarker(BakeParser::StatementEndMarkerContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
  /**/
  /*virtual std::any visitPreposition(BakeParser::PrepositionContext *ctx) override {*/
  /*  return visitChildren(ctx);*/
  /*}*/
};
