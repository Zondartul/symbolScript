#include "util/output.h"
#include "ext/json.hpp"
#include <fstream>
#include <stdexcept>
using json = nlohmann::json;
Output out;

json tok_to_json_helper(Tokenizer::token tok){
    json data = {
            {"type",tok.type},
            {"text",tok.text}
        };
    return data;
}

std::string Output::tokens_to_json(std::vector<Tokenizer::token> tokens){
    json data_tokens;
    for(auto tok:tokens){data_tokens.push_back(tok_to_json_helper(tok));}
    
    json data = {
        {"file_type","tokens"},
        {"data",data_tokens}
    };
    return data.dump();
}

json ast_to_json_helper(Parser::AST ast){
    json data_children;
    for(auto ch:ast.children){
        data_children.push_back(ast_to_json_helper(ch));
    }

    json data = {
        {"tok",tok_to_json_helper(ast.tok)},
        {"children",data_children}
    };

    return data;
}

std::string Output::ast_to_json(Parser::AST ast){
    json data = {
        {"file_type","ast"},
        {"data",ast_to_json_helper(ast)}
    };
    return data.dump();
}

void Output::write_file(std::string filename, std::string data){
    std::ofstream file(filename);
    if(!file.is_open()){throw std::runtime_error(std::string()+"can't write file "+filename);}
    if(!(file << data)){throw std::runtime_error(std::string()+"file write operation failed: "+filename);}
    file.close();
}