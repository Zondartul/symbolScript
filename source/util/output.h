#include "parser/tokenizer.h"
#include "parser/parser.h"

struct Output_impl;

class Output{
    public:
    Output();
    Output_impl *pimpl;

    std::string tokens_to_json(std::vector<Tokenizer::token> tokens);
    std::string ast_to_json(Parser::AST ast);
    void write_file(std::string filename, std::string data);
};

extern Output out;