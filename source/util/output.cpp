#include "util/output.h"
#include "ext/json.hpp"
#include <fstream>
#include <stdexcept>
using json = nlohmann::json;
Output out;
using tok_pos = Tokenizer::tok_pos;

template <typename T> unsigned int get_or_make_idx(std::vector<T>& V, const T& obj){
    for(unsigned int i = 0; i < V.size(); i++){
        if(V.at(i) == obj){return i;}
    }
    V.push_back(obj);
    return V.size()-1;
}

struct Output_impl{
    std::vector<std::string> filenames;
    std::vector<tok_pos> positions;

    void clear(){filenames.clear(); positions.clear();}

    json tok_to_json_helper(Tokenizer::token tok){
        json data = {
                {"type",tok.type},
                {"text",tok.text},
                {"pos1",get_or_make_idx(positions, tok.pos1)},
                {"pos2",get_or_make_idx(positions, tok.pos2)}
            };
        return data;
    }

    json ast_to_json_helper(Parser::AST ast){
        json data_children;
        for(auto ch:ast.children){
            data_children.push_back(ast_to_json_helper(ch));
        }
        json data = {
            {"tok",tok_to_json_helper(ast.tok)},
            {"children",data_children},
        };

        return data;
    }

    json make_dict_data(){
        json js_positions = json::array();
        for(auto pos:positions){
            if(pos.filename.empty()){pos.filename = "(null)";}
            unsigned int filename_idx = get_or_make_idx(filenames,pos.filename);
            json js_pos = {
                {"p",pos.char_idx},
                {"c",pos.col},
                {"l",pos.line},
                {"f",filename_idx}
            };
            js_positions.push_back(js_pos);
        }
        json js_dict = {
            {"filenames",filenames},
            {"positions",js_positions}
        };
        return js_dict;
    }
};

Output::Output(){
    pimpl = new Output_impl();
}


std::string Output::tokens_to_json(std::vector<Tokenizer::token> tokens){
    pimpl->clear();
    json data_tokens;
    for(auto tok:tokens){data_tokens.push_back(pimpl->tok_to_json_helper(tok));}
    
    json data = {
        {"file_type","tokens"},
        {"data",data_tokens},
        {"dict", pimpl->make_dict_data()}
    };

    return data.dump();
}



std::string Output::ast_to_json(Parser::AST ast){
    pimpl->clear();
    json data = {
        {"file_type","ast"},
        {"data",pimpl->ast_to_json_helper(ast)},
        {"dict",pimpl->make_dict_data()},
    };
    return data.dump();
}

void Output::write_file(std::string filename, std::string data){
    std::ofstream file(filename);
    if(!file.is_open()){throw std::runtime_error(std::string()+"can't write file "+filename);}
    if(!(file << data)){throw std::runtime_error(std::string()+"file write operation failed: "+filename);}
    file.close();
}