#include <iostream>
#include <fstream>
#include <sstream>
#include "antlr4-runtime.h"
#include "parser/generated/BakeLexer.h"
#include "parser/generated/BakeParser.h"
#include "parser/TypeCheckVisitor.h"

using namespace antlr4;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./bake <file.bake>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);

    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();

    ANTLRInputStream stream(input);
    BakeLexer lexer(&stream);
    CommonTokenStream tokens(&lexer);
    BakeParser parser(&tokens);

    tree::ParseTree* tree = parser.program();
    TypeCheckVisitor* visitor = new TypeCheckVisitor();

    visitor->visit(tree);

    // std::cout << "Parse tree: " << tree->toStringTree(&parser) << std::endl;
    return 0;
}
