grammar BakeParser;

program: name ingredients? equipment? instructions? EOF;

name: Hash ID;

ingredients: ingredientsHeading item+;
ingredientsHeading: Hash Hash Ingredient
                  | Hash Hash Ingredients
				          ;

equipment: equipmentHeading item+;
equipmentHeading: Hash Hash Equipment;

item: Step itemDecl;
itemDecl: ID LParen type RParen;

type: intType
    | stringType
    | arrayType
	  ;

intType: Int;
stringType: StringLiteral;
arrayType: Int StringLiteral;

instructions: instructionsHeading instruction+;
instructionsHeading: Hash Hash Instructions;
instruction: Step opStmt statementEndMarker
           | Step loopStmt
		       | Step ifStmt statementEndMarker
		       ;

opStmt: Add value (preposition value)?
      | Add value And value (preposition value)?
      | Remove value From value (preposition ID)?
	    | Multiply value By value (preposition ID)?
	    | Split value Into value (preposition ID)?
	    | value Mod value (preposition ID)?
	    | Serve value
	    ;
ifStmt: If binaryExpr Colon opStmt;
binaryExpr: value conditionOp value
          | binaryExpr And binaryExpr
		      | binaryExpr Or binaryExpr
		      ;
loopStmt: Repeat Until binaryExpr Colon instruction+ Step Repeat statementEndMarker;
value: ID
     | ID ArrayAccess ID;
     | ID ArrayAccess Int;
     | Int
	   | StringLiteral
	   ;
conditionOp: Is
           | Is Not
           | LessThan
           | GreaterThan
		       | LessThanEq
		       | GreaterThanEq
		       ;
statementEndMarker: Exclamation
                  | Question
				          | Exclamation statementEndMarker
				          | Question statementEndMarker
				          ;
preposition: Aboard
		       | Above
		       | Across
			     | After
			     | Against
			     | Aloft
			     | Along
			     | Alongside
			     | Amid
			     | Amidst
			     | Mid
			     | Midst
			     | Among
			     | Amongst
			     | Around
			     | Round
			     | Aslant
			     | Astride
			     | At
			     | Atop
			     | Ontop
			     | Before
			     | Behind
			     | Below
			     | Beneath
			     | Neath
			     | Beside
			     | Besides
			     | Beyond
			     | By
			     | Circa
			     | Down
			     | Following
			     | In
			     | Inside
			     | Into
			     | Near
			     | Next
			     | Off
			     | On
			     | Onto
			     | Opposite
			     | Out
			     | Outside
			     | Over
			     | Past
			     | Through
			     | Throughout
			     | To
			     | Touching
			     | Toward
			     | Towards
			     | Under
			     | Underneath
			     | Unto
			     | Up
			     | Upon
			     | With
			     | Within
           ;
