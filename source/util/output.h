#include "parser/tokenizer.h"
#include "parser/parser.h"

class Output{
    public:
    Output() = default;
    static std::string tokens_to_json(std::vector<Tokenizer::token> tokens);
    static std::string ast_to_json(Parser::AST ast);
    static void write_file(std::string filename, std::string data);
};

extern Output out;