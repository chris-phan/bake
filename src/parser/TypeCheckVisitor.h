#include "generated/BakeBaseVisitor.h"
#include "generated/BakeLexer.h"
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

    void assertVariableExists(antlr4::ParserRuleContext *ctx, std::string varName) {
      if (variableTypes.find(varName) == variableTypes.end()) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": "
                  << varName
                  << " hasn't been declared in the ingredients or equipment section"
                  << std::endl;
        exit(1);
      }
    }

    std::string getKindName(Kind k) {
      switch (k) {
        case Kind::Int:
          return "Int";
        case Kind::String:
          return "String";
        case Kind::ArrayInt:
          return "Int[]";
        case Kind::ArrayString:
          return "String[]";
        default:
          return "Unknown";
      }
    }
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

      if (operand1 != operand2) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: Can't add different types, "
                  << getKindName(operand1)
                  << " and "
                  << getKindName(operand2)
                  << std::endl;
      } else if (operand1 != destination) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: Can't assign "
                  << getKindName(operand1)
                  << " to "
                  << getKindName(destination)
                  << std::endl;
      }
    } else {
      Kind destination = std::any_cast<Kind>(visit(ctx->value(1)));
      if (operand1 != destination) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: Can't add "
                  << getKindName(operand1)
                  << " to "
                  << getKindName(destination)
                  << std::endl;
      }
    }
    return visitChildren(ctx);
  }

  virtual std::any visitOpSub(BakeParser::OpSubContext *ctx) override {
    Kind toRemoveType = std::any_cast<Kind>(visit(ctx->value(0)));
    Kind removeFromType = std::any_cast<Kind>(visit(ctx->value(1)));
    if (ctx->preposition()) {
      // Specified destination
      std::string destName = ctx->ID()->getText();
      assertVariableExists(ctx, destName);

      Kind destinationType = variableTypes[destName];
      if (destinationType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->ID()->getText()
                  << " to be type Int but is "
                  << getKindName(destinationType)
                  << " instead"
                  << std::endl;
      }
    }
    if (toRemoveType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->value(0)->getText()
                  << " to be type Int but is "
                  << getKindName(toRemoveType)
                  << " instead"
                  << std::endl;
    } else if (removeFromType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->value(1)->getText()
                  << " to be type Int but is "
                  << getKindName(removeFromType)
                  << " instead"
                  << std::endl;
    }
    return visitChildren(ctx);
  }

  virtual std::any visitOpMult(BakeParser::OpMultContext *ctx) override {
    Kind toMultiplyType = std::any_cast<Kind>(visit(ctx->value(0)));
    Kind multiplierType = std::any_cast<Kind>(visit(ctx->value(1)));
    if (ctx->preposition()) {
      // Specified destination
      std::string destName = ctx->ID()->getText();
      assertVariableExists(ctx, destName);

      Kind destinationType = variableTypes[destName];
      if (destinationType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->ID()->getText()
                  << " to be type Int but is "
                  << getKindName(destinationType)
                  << " instead"
                  << std::endl;
      }
    }
    if (toMultiplyType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->value(0)->getText()
                  << " to be type Int but is "
                  << getKindName(toMultiplyType)
                  << " instead"
                  << std::endl;
    } else if (multiplierType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->value(1)->getText()
                  << " to be type Int but is "
                  << getKindName(multiplierType)
                  << " instead"
                  << std::endl;
    }
    return visitChildren(ctx);
  }

  virtual std::any visitOpDiv(BakeParser::OpDivContext *ctx) override {
    Kind dividendType = std::any_cast<Kind>(visit(ctx->value(0)));
    Kind divisorType = std::any_cast<Kind>(visit(ctx->value(1)));
    if (ctx->preposition()) {
      // Specified destination
      std::string destName = ctx->ID()->getText();
      assertVariableExists(ctx, destName);

      Kind destinationType = variableTypes[destName];
      if (destinationType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->ID()->getText()
                  << " to be type Int but is "
                  << getKindName(destinationType)
                  << " instead"
                  << std::endl;
      }
    }
    if (dividendType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->value(0)->getText()
                  << " to be type Int but is "
                  << getKindName(dividendType)
                  << " instead"
                  << std::endl;
    } else if (divisorType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->value(1)->getText()
                  << " to be type Int but is "
                  << getKindName(divisorType)
                  << " instead"
                  << std::endl;
    }

    if (divisorType == Kind::Int && ctx->value(1)->getText() == "0") {
      std::cerr << "Error: Line "
                << ctx->getStart()->getLine()
                << ": divide by 0 error"
                << std::endl;
    }

    return visitChildren(ctx);
  }

  virtual std::any visitOpMod(BakeParser::OpModContext *ctx) override {
    Kind baseValueType = std::any_cast<Kind>(visit(ctx->value(0)));
    Kind modByType = std::any_cast<Kind>(visit(ctx->value(1)));

    std::string destName = ctx->ID()->getText();
    assertVariableExists(ctx, destName);

    Kind destinationType = variableTypes[destName];
    if (destinationType != Kind::Int) {
      std::cerr << "Error: Line "
                << ctx->getStart()->getLine()
                << ": Type mismatch: expected "
                << ctx->ID()->getText()
                << " to be type Int but is "
                << getKindName(destinationType)
                << " instead"
                << std::endl;
    }

    if (baseValueType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->value(0)->getText()
                  << " to be type Int but is "
                  << getKindName(baseValueType)
                  << " instead"
                  << std::endl;
    } else if (modByType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": Type mismatch: expected "
                  << ctx->value(1)->getText()
                  << " to be type Int but is "
                  << getKindName(modByType)
                  << " instead"
                  << std::endl;
    }

    if (modByType == Kind::Int && ctx->value(1)->getText() == "0") {
      std::cerr << "Error: Line "
                << ctx->getStart()->getLine()
                << ": modulo by 0 error"
                << std::endl;
    }
    return visitChildren(ctx);
  }

  virtual std::any visitOpServe(BakeParser::OpServeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStmt(BakeParser::IfStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBinaryExprCond(BakeParser::BinaryExprCondContext *ctx) override {
    // Enforce <, <=, >, and >= to use Ints
    auto condType = ctx->conditionOp()->getStart()->getType();
    std::vector<int> intComparisons = { BakeLexer::LessThan, BakeLexer::LessThanEq, BakeLexer::GreaterThan, BakeLexer::GreaterThanEq };

    if (std::find(intComparisons.begin(), intComparisons.end(), condType) != intComparisons.end()) {
      Kind lhsType = std::any_cast<Kind>(visit(ctx->value(0)));
      Kind rhsType = std::any_cast<Kind>(visit(ctx->value(1)));
      if (lhsType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": '"
                  << ctx->conditionOp()->getText()
                  << "' is unsupported for type "
                  << getKindName(lhsType)
                  << std::endl;
      } else if (rhsType != Kind::Int) {
        std::cerr << "Error: Line "
                  << ctx->getStart()->getLine()
                  << ": '"
                  << ctx->conditionOp()->getText()
                  << "' is unsupported for type "
                  << getKindName(rhsType)
                  << std::endl;
      }
    }

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
    std::string varName = ctx->ID()->getText();
    assertVariableExists(ctx, varName);
    return variableTypes[varName];
  }

  // Arrays can only be accessed using integers or variables that are integers
  // Strings can't be indexed
  virtual std::any visitValueArrayAccessID(BakeParser::ValueArrayAccessIDContext *ctx) override {
    // Ensure that the variables used in the array access exist
    std::string arrayName = ctx->ID(0)->getText();
    std::string accessIDName = ctx->ID(0)->getText();
    assertVariableExists(ctx, arrayName);
    assertVariableExists(ctx, accessIDName);

    // Ensure that the variable being indexed is an actual array type
    Kind arrayType = variableTypes[arrayName];
    if (arrayType != Kind::ArrayInt || arrayType != Kind::ArrayString) {
      std::cerr << "Error: Can't index " << arrayName << " which is of type " << getKindName(arrayType) << std::endl;
      exit(1);
    }

    // Ensure that the ID used as an index is an Int
    Kind accessIDType = variableTypes[accessIDName];
    if (accessIDType != Kind::Int) {
      std::cerr << "Error: " << accessIDName << " is of type " << getKindName(accessIDType) << " but has to be of type Int" << std::endl;
      exit(1);
    }

    // Expression resolves to a single Int or a single String
    if (arrayType == Kind::ArrayInt) {
      return Kind::Int;
    }
    return Kind::String;
  }

  virtual std::any visitValueArrayAccessInt(BakeParser::ValueArrayAccessIntContext *ctx) override {
    // Ensure that the variable being indexed exists
    std::string arrayName = ctx->ID()->getText();
    assertVariableExists(ctx, arrayName);

    // Ensure that the variable being indexed is an actual array type
    Kind arrayType = variableTypes[arrayName];
    if (arrayType != Kind::ArrayInt || arrayType != Kind::ArrayString) {
      std::cerr << "Error: Can't index " << arrayName << " which is of type " << getKindName(arrayType) << std::endl;
      exit(1);
    }

    // Expression resolves to a single Int or a single String
    if (arrayType == Kind::ArrayInt) {
      return Kind::Int;
    }
    return Kind::String;
  }

  virtual std::any visitValueInt(BakeParser::ValueIntContext *ctx) override {
    return Kind::Int;
  }

  virtual std::any visitValueString(BakeParser::ValueStringContext *ctx) override {
    return Kind::String;
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
