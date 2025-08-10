# Generates the antlr4 lexer, parser, listener, and visitor cpp files

ANTLR_JAR=/usr/local/bin/antlr-4.13.2-complete.jar
GRAMMAR_DIR=src/parser
LEXER=BakeLexer.g4
PARSER=Bake.g4
OUT_DIR=generated
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Go to root dir and then to src/parser
cd $SCRIPT_DIR
cd ..
cd $GRAMMAR_DIR

java -jar $ANTLR_JAR \
  -Dlanguage=Cpp \
  -visitor \
  -o $OUT_DIR \
  $LEXER

java -jar $ANTLR_JAR \
  -Dlanguage=Cpp \
  -visitor \
  -o $OUT_DIR \
  $PARSER
