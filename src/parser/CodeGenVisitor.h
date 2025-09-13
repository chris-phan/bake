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

  // Current antlr lexer rule for string literals converts strings like
  // "hello\n" to "hello\\n"
  // But we want chars like \n, \r, and \t to be preserved
  std::string allowSpecialChars(std::string input) {
    std::string res = "";
    for (size_t i = 0; i < input.length(); ++i) {
      if (input[i] == '\\') {
        switch (input[i + 1]) {
          case '"':
            res += '"';
            break;
          case '\\':
            res += '\\';
            break;
          case 'n':
            res += '\n';
            break;
          case 'r':
            res += '\r';
            break;
          case 't':
            res += '\t';
            break;
          default:
            res += '\\' + input[i + 1];
            break;
        }

        i++;
      } else {
        res += input[i];
      }
    }

    return res;
  }

  Value* loadStrID(Value *val) {
    auto *i8PtrTy = PointerType::get(Type::getInt8Ty(*theContext), 0);
    return builder->CreateLoad(i8PtrTy, val, "loadStrID");
  }

  Value* loadIfID(BakeParser::ValueContext *ctx, Value *val) {
    // Don't load if it's an Int literal or String literal
    if (dynamic_cast<BakeParser::ValueIntContext*>(ctx) != nullptr) {
      return val;
    } else if (dynamic_cast<BakeParser::ValueStringContext*>(ctx) != nullptr) {
      return val;
    }

    if (dynamic_cast<BakeParser::ValueIDContext*>(ctx) != nullptr) {
      std::string id = ctx->getText();
      Kind valType = variableTypes[id];
      if (valType == Kind::Int) {
        return builder->CreateLoad(builder->getInt32Ty(), val, "loadID");
      } else if (valType == Kind::String) {
        return loadStrID(val);
      }
    }

    std::string arrName;
    if (auto arrCtx = dynamic_cast<BakeParser::ValueArrayAccessIDContext*>(ctx)) {
      arrName = arrCtx->ID(0)->getText();
    } else if (auto arrCtx = dynamic_cast<BakeParser::ValueArrayAccessIntContext*>(ctx)) {
      arrName = arrCtx->ID()->getText();
    }

    Kind valType = variableTypes[arrName];
    if (valType == Kind::ArrayInt) {
      return builder->CreateLoad(builder->getInt32Ty(), val, "loadArrayAccessID");
    } else {
      return loadStrID(val);
    }

    return val;
  }

  FunctionCallee getStrlen() {
    return theModule->getOrInsertFunction(
      "strlen",
      FunctionType::get(
        builder->getInt64Ty(),
        {PointerType::get(builder->getInt8Ty(), 0)},
        false
      )
    );
  }

  FunctionCallee getMalloc() {
    return theModule->getOrInsertFunction(
      "malloc",
      FunctionType::get(
        PointerType::get(builder->getInt8Ty(), 0),
        {builder->getInt64Ty()},
        false
      )
    );
  }

  FunctionCallee getMemcpy() {
    return theModule->getOrInsertFunction(
      "memcpy",
      FunctionType::get(
        PointerType::get(builder->getInt8Ty(), 0),
        {
          PointerType::get(builder->getInt8Ty(), 0),
          PointerType::get(builder->getInt8Ty(), 0),
          builder->getInt64Ty()
        },
        false
      )
    );
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

  // To initialize a string, create a global array that holds the actual characters
  // and then a global ptr to that array, the global ptr is what gets loaded
  // and interacted with
  // Makes mutation simpler
  virtual std::any visitStringType(BakeParser::StringTypeContext *ctx) override {
    std::string strVal = ctx->StringLiteral()->getText();

    // Trim starting and end quotes ("")
    strVal = strVal.substr(1, strVal.length() - 2);
    strVal = allowSpecialChars(strVal);

    auto strConst = ConstantDataArray::getString(*theContext, strVal, true);
    GlobalVariable *globalStrData = new GlobalVariable(
      *theModule,
      strConst->getType(),
      false,
      GlobalValue::PrivateLinkage,
      strConst,
      "stringData"
    );

    auto zero = ConstantInt::get(Type::getInt32Ty(*theContext), 0);
    Constant *strPtr = ConstantExpr::getInBoundsGetElementPtr(
      strConst->getType(),
      globalStrData,
      ArrayRef<Constant*>({zero, zero})
    );

    auto ptrTy = PointerType::get(Type::getInt8Ty(*theContext), 0);

    Value *gv = new GlobalVariable(
      *theModule,
      ptrTy,
      false,
      GlobalValue::PrivateLinkage,
      strPtr,
      "stringPtr"
    );

    return std::make_pair(gv, Kind::String);
  }

  virtual std::any visitArrayInt(BakeParser::ArrayIntContext *ctx) override {
    int arrLen = std::stoi(ctx->Int(0)->getText());
    int initVal = std::stoi(ctx->Int(1)->getText());
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

  virtual std::any visitArrayIntLiteral(BakeParser::ArrayIntLiteralContext *ctx) override {
    int arrLen = ctx->Int().size();
    ArrayType *arrType = ArrayType::get(builder->getInt32Ty(), arrLen);
    std::vector<Constant*> initVals;
    for (int i = 0; i < arrLen; ++i) {
      int val = std::stoi(ctx->Int(i)->getText());
      initVals.push_back(ConstantInt::get(builder->getInt32Ty(), val));
    }

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
    initVal = allowSpecialChars(initVal);

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
    Value *operand1 = loadIfID(ctx->value(0), operand1Info.first);
    Value *destination = namedValues[ctx->ID()->getText()];

    Value *addResult;
    if (operand1Info.second == Kind::String) {
      FunctionCallee strlenFunc = getStrlen();
      FunctionCallee mallocFunc = getMalloc();
      FunctionCallee memcpyFunc = getMemcpy();

      Value *lhs = nullptr;
      Value *rhs = nullptr;
      if (ctx->And()) {
        std::pair<Value*, Kind> operand2Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(1)));
        lhs = operand1;
        rhs = loadIfID(ctx->value(1), operand2Info.first);
      } else {
        lhs = loadStrID(destination);
        rhs = operand1;
      }
       
      Value *lhsLen = builder->CreateCall(strlenFunc, {lhs});
      Value *rhsLen = builder->CreateCall(strlenFunc, {rhs});

      // Add 1 to make room for null char '\0'
      Value *concatLen = builder->CreateAdd(
        builder->CreateAdd(lhsLen, rhsLen),
        builder->getInt64(1),
        "addStrLens"
      );

      Value *buffer = builder->CreateCall(mallocFunc, {concatLen});
      builder->CreateCall(memcpyFunc, {buffer, lhs, lhsLen});

      Value *idxToStoreOp2 = builder->CreateInBoundsGEP(builder->getInt8Ty(), buffer, lhsLen);
      builder->CreateCall(memcpyFunc, {idxToStoreOp2, rhs, rhsLen});

      Value *nullTermIdx = builder->CreateInBoundsGEP(builder->getInt8Ty(), buffer, builder->CreateSub(concatLen, builder->getInt64(1)));
      builder->CreateStore(builder->getInt8(0), nullTermIdx);

      addResult = buffer;
    } else {
      if (ctx->And()) {
        std::pair<Value*, Kind> operand2Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(1)));
        Value *operand2 = loadIfID(ctx->value(1), operand2Info.first);
        addResult = builder->CreateAdd(operand1, operand2, "addInt");
      } else {
        Value *loadedDest = builder->CreateLoad(builder->getInt32Ty(), destination, "loadID");
        addResult = builder->CreateAdd(operand1, loadedDest, "addInt");
      }
    }
    builder->CreateStore(addResult, destination);

    return nullptr;
  }

  virtual std::any visitOpSub(BakeParser::OpSubContext *ctx) override {
    std::pair<Value*, Kind> operand1Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(0)));
    std::pair<Value*, Kind> operand2Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(1)));

    // Syntax is `remove op1 from op2`, so do op2 - op1
    Value *operand1 = loadIfID(ctx->value(0), operand1Info.first);
    Value *operand2 = loadIfID(ctx->value(1), operand2Info.first);
    if (ctx->preposition()) {
      Value *destination = namedValues[ctx->ID()->getText()];

      Value *subResult = builder->CreateSub(operand2, operand1, "subInt");
      builder->CreateStore(subResult, destination);
    } else {
      Value *subResult = builder->CreateSub(operand2, operand1, "subInt");
      builder->CreateStore(subResult, operand2Info.first);
    }

    return nullptr;
  }

  virtual std::any visitOpMult(BakeParser::OpMultContext *ctx) override {
    std::pair<Value*, Kind> operand1Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(0)));
    std::pair<Value*, Kind> operand2Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(1)));

    Value *operand1 = loadIfID(ctx->value(0), operand1Info.first);
    Value *operand2 = loadIfID(ctx->value(1), operand2Info.first);
    if (ctx->preposition()) {
      Value *destination = namedValues[ctx->ID()->getText()];

      Value *multResult = builder->CreateMul(operand1, operand2, "multInt");
      builder->CreateStore(multResult, destination);
    } else {
      Value *multResult = builder->CreateMul(operand1, operand2, "multInt");
      builder->CreateStore(multResult, operand1Info.first);
    }

    return nullptr;
  }

  virtual std::any visitOpDiv(BakeParser::OpDivContext *ctx) override {
    std::pair<Value*, Kind> operand1Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(0)));
    std::pair<Value*, Kind> operand2Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(1)));

    Value *operand1 = loadIfID(ctx->value(0), operand1Info.first);
    Value *operand2 = loadIfID(ctx->value(1), operand2Info.first);
    if (ctx->preposition()) {
      Value *destination = namedValues[ctx->ID()->getText()];

      Value *divResult = builder->CreateSDiv(operand1, operand2, "divInt");
      builder->CreateStore(divResult, destination);
    } else {
      Value *divResult = builder->CreateSDiv(operand1, operand2, "divInt");
      builder->CreateStore(divResult, operand1Info.first);
    }

    return nullptr;
  }

  virtual std::any visitOpMod(BakeParser::OpModContext *ctx) override {
    std::pair<Value*, Kind> operand1Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(0)));
    std::pair<Value*, Kind> operand2Info = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(1)));

    Value *operand1 = loadIfID(ctx->value(0), operand1Info.first);
    Value *operand2 = loadIfID(ctx->value(1), operand2Info.first);
    Value *destination = namedValues[ctx->ID()->getText()];

    Value *modResult = builder->CreateSRem(operand1, operand2, "modInt");
    builder->CreateStore(modResult, destination);

    return nullptr;
  }

  virtual std::any visitOpServe(BakeParser::OpServeContext *ctx) override {
    FunctionType *printfType = FunctionType::get(
      builder->getInt32Ty(),
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
      auto *fmtConst = ConstantDataArray::getString(*theContext, "%s", true);
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
      return builder->CreateCall(printfFunc, {formatStr, val});
    }

    return nullptr;
  }

  virtual std::any visitIfStmt(BakeParser::IfStmtContext *ctx) override {
    Value *condition = std::any_cast<Value*>(visit(ctx->binaryExpr()));

    Function *func = builder->GetInsertBlock()->getParent();
    BasicBlock *thenBlock = BasicBlock::Create(*theContext, "then", func);
    BasicBlock *mergeBlock = BasicBlock::Create(*theContext, "merge", func);

    builder->CreateCondBr(condition, thenBlock, mergeBlock);
    builder->SetInsertPoint(thenBlock);
    visit(ctx->opStmt());
    builder->CreateBr(mergeBlock);

    builder->SetInsertPoint(mergeBlock);

    return nullptr;
  }

  virtual std::any visitBinaryExprCond(BakeParser::BinaryExprCondContext *ctx) override {
    auto condOp = ctx->conditionOp();
    std::pair<Value*, Kind> lhsInfo = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(0)));
    std::pair<Value*, Kind> rhsInfo = std::any_cast<std::pair<Value*, Kind>>(visit(ctx->value(1)));
    Value *lhs = loadIfID(ctx->value(0), lhsInfo.first);
    Value *rhs = loadIfID(ctx->value(1), rhsInfo.first);

    Value *cmp = nullptr;
    if (dynamic_cast<BakeParser::CondOpIsContext*>(condOp) != nullptr) {
      cmp = builder->CreateICmpEQ(lhs, rhs);
    } else if (dynamic_cast<BakeParser::CondOpIsNotContext*>(condOp) != nullptr) {
      cmp = builder->CreateICmpNE(lhs, rhs);
    } else if (dynamic_cast<BakeParser::CondOpLessThanContext*>(condOp) != nullptr) {
      cmp = builder->CreateICmpSLT(lhs, rhs);
    } else if (dynamic_cast<BakeParser::CondOpGreaterThanContext*>(condOp) != nullptr) {
      cmp = builder->CreateICmpSGT(lhs, rhs);
    } else if (dynamic_cast<BakeParser::CondOpLessThanEqContext*>(condOp) != nullptr) {
      cmp = builder->CreateICmpSLE(lhs, rhs);
    } else if (dynamic_cast<BakeParser::CondOpGreaterThanEqContext*>(condOp) != nullptr) {
      cmp = builder->CreateICmpSGE(lhs, rhs);
    }

    return cmp;
  }

  virtual std::any visitBinaryExprOr(BakeParser::BinaryExprOrContext *ctx) override {
    Value *lhs = std::any_cast<Value*>(visit(ctx->binaryExpr(0)));
    Value *rhs = std::any_cast<Value*>(visit(ctx->binaryExpr(1)));

    Value *orExpr = builder->CreateOr(lhs, rhs, "binaryOr");
    return builder->CreateOr(lhs, rhs, "binaryOr");
  }

  virtual std::any visitBinaryExprAnd(BakeParser::BinaryExprAndContext *ctx) override {
    Value *lhs = std::any_cast<Value*>(visit(ctx->binaryExpr(0)));
    Value *rhs = std::any_cast<Value*>(visit(ctx->binaryExpr(1)));

    return builder->CreateAnd(lhs, rhs, "binaryAnd");
  }

  virtual std::any visitLoopStmt(BakeParser::LoopStmtContext *ctx) override {
    Function *func = builder->GetInsertBlock()->getParent();
    BasicBlock *condBlock = BasicBlock::Create(*theContext, "loopCond", func);
    BasicBlock *loopBody = BasicBlock::Create(*theContext, "loopBody", func);
    BasicBlock *afterBlock = BasicBlock::Create(*theContext, "afterLoop", func);

    builder->CreateBr(condBlock);
    builder->SetInsertPoint(condBlock);

    // Language semantics is repeat until <this condition is true>
    // So, if the condition is false, we need to keep looping
    Value *condition = std::any_cast<Value*>(visit(ctx->binaryExpr()));

    Value *negCond = builder->CreateNot(condition, "loopCondition");

    builder->CreateCondBr(negCond, loopBody, afterBlock);

    builder->SetInsertPoint(loopBody);
    for (auto inst : ctx->instruction()) {
      visit(inst);
    }
    builder->CreateBr(condBlock);

    builder->SetInsertPoint(afterBlock);

    return nullptr;
  }

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
    Value *idx = builder->CreateLoad(builder->getInt32Ty(), namedValues[ctx->ID(1)->getText()]);
    Value *oneAdjustedIdx = builder->CreateSub(idx, one, "adjustIdxBy1");

    // Array access resolves to either an Int or String
    Kind arrType = variableTypes[ctx->ID(0)->getText()];
    Type *llvmElemType = builder->getInt32Ty();
    Kind type = Kind::Int;
    if (arrType == Kind::ArrayString) {
      type = Kind::String;
      llvmElemType = PointerType::get(builder->getInt8Ty(), 0);
    }

    GlobalVariable *arr = dyn_cast<GlobalVariable>(namedValues[ctx->ID(0)->getText()]);
    auto arrLen = dyn_cast<ArrayType>(arr->getValueType())->getNumElements();
    Value *elemPtr = builder->CreateInBoundsGEP(ArrayType::get(builder->getInt32Ty(), arrLen), arr, {zero, oneAdjustedIdx}, "elemPtr");

    return std::make_pair(elemPtr, type);
  }

  virtual std::any visitValueArrayAccessInt(BakeParser::ValueArrayAccessIntContext *ctx) override {
    Value *zero = builder->getInt32(0);

    // The language is 1-indexed, so subtract 1 from the provided idx
    Value *one = builder->getInt32(1);
    Value *idx = builder->getInt32(std::stoi(ctx->Int()->getText()));
    Value *oneAdjustedIdx = builder->CreateSub(idx, one, "adjustIdxBy1");

    // Array access resolves to either an Int or String
    Kind arrType = variableTypes[ctx->ID()->getText()];
    Type *llvmElemType = builder->getInt32Ty();
    Kind type = Kind::Int;
    if (arrType == Kind::ArrayString) {
      type = Kind::String;
      llvmElemType = PointerType::get(builder->getInt8Ty(), 0);
    }

    GlobalVariable *arr = dyn_cast<GlobalVariable>(namedValues[ctx->ID()->getText()]);
    auto arrLen = dyn_cast<ArrayType>(arr->getValueType())->getNumElements();
    Value *elemPtr = builder->CreateInBoundsGEP(ArrayType::get(builder->getInt32Ty(), arrLen), arr, {zero, oneAdjustedIdx}, "elemPtr");

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
    strVal = allowSpecialChars(strVal);

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
