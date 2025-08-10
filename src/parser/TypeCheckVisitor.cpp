#include "generated/BakeBaseVisitor.h"
#include "generated/BakeParser.h"
#include <iostream>

using Value = std::variant<std::string, int, std::vector<std::string>, std::vector<int>>;

class TypeCheckVisitor : public BakeBaseVisitor {
private:
    enum class Kind {
        Int,
        String,
        ArrayInt,
        ArrayString
    };
    std::unordered_map<std::string, Kind> variableTypes;
public:
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
      Kind varType = std::any_cast<Kind>(visit(ctx->type()));
      variableTypes[varName] = varType;
      return nullptr;
  }

  virtual std::any visitType(BakeParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIntType(BakeParser::IntTypeContext *ctx) override {
    return Kind::Int;
  }

  virtual std::any visitStringType(BakeParser::StringTypeContext *ctx) override {
    return Kind::String;
  }

  virtual std::any visitArrayInt(BakeParser::ArrayIntContext *ctx) override {
    return Kind::ArrayInt;
  }

  virtual std::any visitArrayString(BakeParser::ArrayStringContext *ctx) override {
    return Kind::ArrayString;
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
    Kind operand1 = std::any_cast<Kind>(visit(ctx->value(0)));
    if (ctx->And()) {
      Kind operand2 = std::any_cast<Kind>(visit(ctx->value(1)));
      Kind destination = std::any_cast<Kind>(visit(ctx->value(2)));

      if (operand1 != operand2 || operand1 != destination) {
        std::cerr << "Error: Type mismatch" << std::endl;
        std::cerr << ctx << std::endl;
      }
    } else {
      Kind destination = std::any_cast<Kind>(visit(ctx->value(1)));
      if (operand1 != destination) {
        std::cerr << "Error: Type mismatch" << std::endl;
        std::cerr << ctx << std::endl;
      }
    }
    return visitChildren(ctx);
  }

  virtual std::any visitOpSub(BakeParser::OpSubContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOpMult(BakeParser::OpMultContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOpDiv(BakeParser::OpDivContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOpMod(BakeParser::OpModContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOpServe(BakeParser::OpServeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStmt(BakeParser::IfStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBinaryExprCond(BakeParser::BinaryExprCondContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBinaryExprOr(BakeParser::BinaryExprOrContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBinaryExprAnd(BakeParser::BinaryExprAndContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLoopStmt(BakeParser::LoopStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValueID(BakeParser::ValueIDContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValueArrayAccessID(BakeParser::ValueArrayAccessIDContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValueArrayAccessInt(BakeParser::ValueArrayAccessIntContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValueInt(BakeParser::ValueIntContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValueString(BakeParser::ValueStringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCondOpIs(BakeParser::CondOpIsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCondOpIsNot(BakeParser::CondOpIsNotContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCondOpLessThan(BakeParser::CondOpLessThanContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCondOpGreaterThan(BakeParser::CondOpGreaterThanContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCondOpLessThanEq(BakeParser::CondOpLessThanEqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCondOpGreaterThanEq(BakeParser::CondOpGreaterThanEqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatementEndMarker(BakeParser::StatementEndMarkerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPreposition(BakeParser::PrepositionContext *ctx) override {
    return visitChildren(ctx);
  }
};
