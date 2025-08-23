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
  std::map<std::string, Value*> namedValues;
  std::map<std::string, Kind> variableTypes;

  Value* loadIfID(BakeParser::ValueContext *ctx, Value *val) {
    if (dynamic_cast<BakeParser::ValueIDContext*>(ctx) != nullptr) {
      return builder->CreateLoad(builder->getInt32Ty(), val, "loadID");
    }
    // TODO: figure out how to load this other stuff
    /*else if (dynamic_cast<BakeParser::ValueArrayAccessIDContext*>(ctx) != nullptr) {*/
    /*  return builder->CreateLoad(elemType, val, "loadArrayAccessID");*/
    /*} else if (dynamic_cast<BakeParser::ValueArrayAccessIntContext*>(ctx) != nullptr) {*/
    /*  return builder->CreateLoad(elemType, val, "loadArrayAccessInt");*/
    /*}*/

    // Don't load if it's an Int literal or String literal
    return val;
  }

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

  std::unique_ptr<LLVMContext> getContext() {
    return std::move(theContext);
  }

  std::unique_ptr<Module> getModule() {
    return std::move(theModule);
  }

  // Call after visit() to end the main function block
  void finish() {
    builder->CreateRet(builder->getInt32(0));
    /*theModule->print(outs(), nullptr);*/
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
    std::pair<Value*, Kind> varInfo = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->type()));
    namedValues[varName] = varInfo.first;
    variableTypes[varName] = varInfo.second;

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

    return std::make_pair(globalInt, Kind::Int);
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
    Value *strPtr = builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, idxs, "strPtr");
    return std::make_pair(strPtr, Kind::String);
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
    Value *intArrPtr = builder->CreateInBoundsGEP(globalIntArr->getValueType(), globalIntArr, idxs, "intArrPtr");
    return std::make_pair(intArrPtr, Kind::ArrayInt);
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

    Value *strArrPtr = builder->CreateInBoundsGEP(globalStrArr->getValueType(), globalStrArr, {zero, zero}, "strArrPtr");
    return std::make_pair(strArrPtr, Kind::ArrayString);
  }

  virtual std::any visitInstructions(BakeParser::InstructionsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInstructionsHeading(BakeParser::InstructionsHeadingContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInstOpStmt(BakeParser::InstOpStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInstLoopStmt(BakeParser::InstLoopStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInstIfStmt(BakeParser::InstIfStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOpAdd(BakeParser::OpAddContext *ctx) override {
    std::pair<Value*, Kind> operand1Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(0)));
    std::pair<Value*, Kind> operand2Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(1)));

    Value *operand1 = loadIfID(ctx->value(0), operand1Info.first);
    Value *operand2 = loadIfID(ctx->value(1), operand2Info.first);
    if (ctx->And()) {
      std::pair<Value*, Kind> destinationInfo = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(2)));
      Value *destination = destinationInfo.first;

      Value *addResult = builder->CreateAdd(operand1, operand2, "addInt");
      builder->CreateStore(addResult, destination, "storeWithDest");
    } else {
      Value *addResult = builder->CreateAdd(operand1, operand2, "addInt");
      builder->CreateStore(addResult, operand2, "storeInPlace");
    }

    return nullptr;
  }

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

  virtual std::any visitOpServe(BakeParser::OpServeContext *ctx) override {
    FunctionType *printfType = FunctionType::get(
      builder->getInt8Ty(),
      {PointerType::get(builder->getInt8Ty(), 0)},
      true
    );
    FunctionCallee printfFunc = theModule->getOrInsertFunction("printf", printfType);

    Value *formatStr = nullptr;
    std::pair<Value*, Kind> valInfo = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value()));
    Value *val = loadIfID(ctx->value(), valInfo.first);
    Kind type = valInfo.second;

    if (type == Kind::Int) {
      auto *fmtConst = ConstantDataArray::getString(*theContext, "%d", true);
      auto *globalFmt = new GlobalVariable(
        *theModule,
        fmtConst->getType(),
        true,
        GlobalValue::PrivateLinkage,
        fmtConst,
        "fmtStringInt"
      );
      Value *zero = builder->getInt32(0);
      formatStr = builder->CreateInBoundsGEP(globalFmt->getValueType()->getArrayElementType(), globalFmt, {zero, zero}, "fmtPtrInt");
      builder->CreateCall(printfFunc, {formatStr, val});
    } else if (type == Kind::String) {
      formatStr = val;
      return builder->CreateCall(printfFunc, {formatStr});
    }

    return nullptr;
  }

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

  virtual std::any visitValueID(BakeParser::ValueIDContext *ctx) override {
    Value *val = namedValues[ctx->ID()->getText()];
    Kind type = variableTypes[ctx->ID()->getText()];
    return std::make_pair(val, type);
  }

  // Arrays can only be accessed using integers or variables that are integers
  // Strings can't be indexed
  virtual std::any visitValueArrayAccessID(BakeParser::ValueArrayAccessIDContext *ctx) override {
    Value *zero = builder->getInt32(0);

    // The language is 1-indexed, so subtract 1 from the provided idx
    Value *one = builder->getInt32(1);
    Value *idx = namedValues[ctx->ID(1)->getText()];
    Value *oneAdjustedIdx = builder->CreateSub(idx, one, "adjustIdxBy1");

    Value *arr = namedValues[ctx->ID(0)->getText()];
    Type *elemType = cast<PointerType>(arr->getType())->getArrayElementType();
    Value *elemPtr = builder->CreateInBoundsGEP(elemType, arr, oneAdjustedIdx, "elemPtr");

    // Array access resolves to either an Int or String
    Kind arrType = variableTypes[ctx->ID(0)->getText()];
    Kind type = Kind::Int;
    if (arrType == Kind::ArrayString) {
      type = Kind::String;
    }

    return std::make_pair(elemPtr, type);
  }

  virtual std::any visitValueArrayAccessInt(BakeParser::ValueArrayAccessIntContext *ctx) override {
    Value *zero = builder->getInt32(0);

    // The language is 1-indexed, so subtract 1 from the provided idx
    Value *one = builder->getInt32(1);
    Value *idx = builder->getInt32(std::stoi(ctx->Int()->getText()));
    Value *oneAdjustedIdx = builder->CreateSub(idx, one, "adjustIdxBy1");

    Value *arr = namedValues[ctx->ID()->getText()];
    Type *elemType = cast<PointerType>(arr->getType())->getArrayElementType();
    Value *elemPtr = builder->CreateInBoundsGEP(elemType, arr, oneAdjustedIdx, "elemPtr");

    // Array access resolves to either an Int or String
    Kind arrType = variableTypes[ctx->ID()->getText()];
    Kind type = Kind::Int;
    if (arrType == Kind::ArrayString) {
      type = Kind::String;
    }

    return std::make_pair(elemPtr, type);
  }

  virtual std::any visitValueInt(BakeParser::ValueIntContext *ctx) override {
    Value *val = builder->getInt32(std::stoi(ctx->Int()->getText()));
    return std::make_pair(val, Kind::Int);
  }

  virtual std::any visitValueString(BakeParser::ValueStringContext *ctx) override {
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
    Value *strPtr = builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, idxs, "strPtr");
    return std::make_pair(strPtr, Kind::String);
  }

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
