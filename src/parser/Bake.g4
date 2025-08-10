grammar Bake;

options {
  tokenVocab=BakeLexer;
}

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
arrayType: Int Int            #arrayInt
         | Int StringLiteral  #arrayString
				 ;

instructions: instructionsHeading instruction+;
instructionsHeading: Hash Hash Instructions;
instruction: Step opStmt statementEndMarker  #instOpStmt
           | Step loopStmt                   #instLoopStmt
		       | Step ifStmt statementEndMarker  #instIfStmt
		       ;

opStmt: Add value (And value)? preposition value   #opAdd
      | Remove value From value (preposition ID)?  #opSub
	    | Multiply value By value (preposition ID)?  #opMult
	    | Split value Into value (preposition ID)?   #opDiv
	    | value Mod value (preposition ID)?          #opMod
	    | Serve value                                #opServe
	    ;
ifStmt: If binaryExpr Colon opStmt;
binaryExpr: value conditionOp value    #binaryExprCond
          | binaryExpr And binaryExpr  #binaryExprAnd
		      | binaryExpr Or binaryExpr   #binaryExprOr
		      ;
loopStmt: Repeat Until binaryExpr Colon instruction+ Step Repeat statementEndMarker;
value: ID                  #valueID
     | ID ArrayAccess ID   #valueArrayAccessID
     | ID ArrayAccess Int  #valueArrayAccessInt
     | Int                 #valueInt
	   | StringLiteral       #valueString
	   ;
conditionOp: Is             #condOpIs
           | Is Not         #condOpIsNot
           | LessThan       #condOpLessThan
           | GreaterThan    #condOpGreaterThan
		       | LessThanEq     #condOpLessThanEq
		       | GreaterThanEq  #condOpGreaterThanEq
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
