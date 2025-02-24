#include "tokenizer.h"
#include <iostream>
#include <fstream>

/// for errors
std::string *cur_file_text = nullptr;


/// --------- miniTokenizer ------------
typedef miniTokenizer::token                    minitoken;
typedef std::vector<miniTokenizer::token>       v_minitokens;
typedef miniTokenizer::token_info               minitoken_info;
typedef std::vector<miniTokenizer::token_info>  v_minitoken_infos;

v_minitokens miniTokenizer::tokenize(const std::string &S, v_minitoken_infos tok_dict){
    v_minitokens tokens;
    
    std::smatch match;
    auto flags = std::regex_constants::match_continuous;
    int line_num = 0;

    for(auto I = S.cbegin(); I != S.end(); I += match.str().length()){ //while(I != S.cend()){
        bool found = false;
        for(const auto &ti:tok_dict){
            bool res = std::regex_search(I, S.end(), match, ti.reg, flags);
            if(!res){continue;}

            minitoken tok;
            tok.type = ti.type;
            tok.text = match.str();
            found = true;
            //std::cout << "matched " << tok.type << " [" << tok.text << "]" << std::endl;
            tokens.push_back(tok);
            if(tok.text.find('\n') != -1){line_num++;}
            break;
        }
        if(!found){
            int pos = std::distance(S.begin(), I);
            std::stringstream maybe_tok;
            maybe_tok << "[" << S.substr(pos,10) << "...]";
            std::cout << "Tokenizer error @ line " << line_num << ":\n";
            std::cout << "Can't read token starting from " << maybe_tok.str();
            //std::cout << *I << "]: " << S.substr(std::distance(S.begin(),I)) << std::endl;
            std::cout << "(( position = " << pos << "))" << std::endl;
            throw std::runtime_error("can't read token "+maybe_tok.str());
        }
    }
    return tokens;
}
///---------------- FileStore ------------------------------
FileStore fileStore;

std::string& FileStore::open_file(std::string filename){
    if(files.count(filename)){return files.at(filename);}
    
    auto &file = files[filename];
    std::ifstream fs_file(filename);
    if(!fs_file.is_open()){throw std::runtime_error("can't open file " + filename);}
    getline(fs_file, file, '\0');
    
    return file;
}


///---------------- Tokenizer ------------------------------
std::string Tokenizer::cur_filename;

typedef Tokenizer::tok_pos                  tok_pos;
typedef Tokenizer::token                    token;
typedef std::vector<Tokenizer::token>       v_tokens;

bool tok_pos::operator<(const tok_pos& other) const{
    if(filename < other.filename){return true;}
    else if(filename == other.filename){
        if(line < other.line){return true;}
        else if(line == other.line){
            if(col < other.col){return true;}
            else if(col == other.col){
                if(char_idx < other.char_idx){return true;}
            }
        }
    }
    return false;
}

bool token::operator==(const token& other) const{
    if((text != "") && (text[0] == '\\')){
        // backslash means this is an 'exact text' token.
        std::string text2 = text.substr(1);
        return text2 == other.text;
    }else{
        return type == other.type;
    }
}

bool token::operator<(const token& other) const{
    if(pos1 < other.pos1){return true;}
    else if(pos1 == other.pos1){
        if(pos2 < other.pos2){return true;}
        else if(pos2 == other.pos2){
            if(type < other.type){return true;}
            else if(type == other.type){
                if(text < other.text){return true;}
            }
        }
    }
    return false;
}

Tokenizer::Tokenizer(){}
Tokenizer::~Tokenizer(){
    if(cur_file_text == &file){cur_file_text = nullptr;}
}

void Tokenizer::reset(){
    //std::string file;
    //std::vector<token> tokens;
    //static std::string cur_filename; ///so like, why even make things static if we gonna do this anyway?
    file.clear();
    tokens.clear();
    cur_filename.clear();
}

void Tokenizer::open_file(std::string filename){
    file = fileStore.open_file(filename);
    cur_file_text = &file;
    cur_filename = filename;
}

void Tokenizer::set_text(std::string text){
    file = text;
    cur_file_text = &file;
    cur_filename = "(no file)";
}

std::vector<Tokenizer::token> Tokenizer::annotate_positions(const std::vector<miniTokenizer::token>& tokens){
    std::vector<token> res;
    tok_pos cur_pos;
    cur_pos.filename = cur_filename;
    for(const auto& tok: tokens){
        token tok2;
        tok2.type = tok.type;
        tok2.text = tok.text;
        tok2.pos1 = cur_pos;
        int L = tok.text.length();
        cur_pos.char_idx += L;
        cur_pos.col += L;
        //if(tok.text == "\n"){ /// matches the newline token
        if(tok.text.size() && (tok.text.back() == '\n')){ /// matches any token that ends with newline
            cur_pos.line += 1;
            cur_pos.col = 0;
        }
        tok2.pos2 = cur_pos;
    
        res.push_back(tok2);
    }
    return res;
}

std::vector<token> Tokenizer::erase_token_type(
        const std::vector<token>& tokens, 
        std::set<std::string> erased_types)
{
    #define IS_ERASED(x) erased_types.count(tok.type)

    std::vector<token> res;
    for(auto tok:tokens){
        if(!IS_ERASED(tok.type)){res.push_back(tok);}
    }
    return res;
}

void Tokenizer::print_tokens(std::vector<token> tokens){
    int line = 0;
    for(auto tok: tokens){
        if(tok.pos1.line != line)
        {line = tok.pos1.line; std::cout << std::endl;}
        std::cout << "["<<tok.text<<"]";
    }
    std::cout << std::endl;
}

void Tokenizer::load(){
    const std::string S = file;
    miniTokenizer MT;
    auto mt_tokens = MT.tokenize(S, tok_dict);
    tokens = annotate_positions(mt_tokens);
    
    /// 2024.10.31 - temporarily disable skipped tokens
    ///  so we can see them in LazyCompGUI
    //tokens = erase_token_type(tokens, {"COMMENT", "SP", "NL"});
}


bool operator==(const Tokenizer::tok_pos& pos1, const Tokenizer::tok_pos& pos2){
    return (pos1.char_idx == pos2.char_idx) &&
    (pos1.col == pos2.col) &&
    (pos1.line == pos2.line) &&
    (pos1.filename == pos2.filename);
}