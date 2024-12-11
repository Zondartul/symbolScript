#include "parser/parse_template.h"
#include "parser/span.h"
#include <iostream>
using rule = MiniParser::rule;
using AST = Parser::AST;

Parser::AST ast(std::string name, std::vector<Parser::AST> children){
    Parser::AST res;
    if(name.at(0) == '\\'){
        res.tok.text = name.substr(1); // 2024.12.11 i think we only construct these for parse_template
    }else{
        res.tok.type = name;
    }
    res.children = children;
    return res;
}

Parser::AST ast_literal(std::string text){
    auto new_ast = ast("");
    new_ast.tok.text = text;
    return new_ast;
}

bool rmt_debug = false;

template<typename T> std::string print_list(T& list, std::string delim=", "){
    std::stringstream ss;
    for(auto I = list.begin(); I != list.end(); I++){
        auto &val = *I; bool last = (I+1) == list.end();
        ss << val;
        if(!last){ss << delim;}
    }
    return ss.str();
}

std::ostream& operator<<(std::ostream& stream, rule R){
    stream << "[" << print_list(R.previous, " ") << " . " << R.lookahead << " -> " << R.tok << "]";
    return stream;
}

bool rule_matches_template(rule R, AST templ){
    Span<std::string> rule_span(R.previous);
    Span<AST> templ_span(templ.children);
    if(rmt_debug){
        std::cout << "rule_matches_template? " << R << " -> " << templ << std::endl;
    }
    bool res = (templ == R.tok) && (templ_span == rule_span);
    
    return res;
}

// -1 means SHIFT next token onto stack
// 0..N means REDUCE using rule N
const int RULE_IDX_ERROR = -100;
const int RULE_IDX_SHIFT = -1;

int get_rule_index(const std::vector<rule>& rules, const AST& templ){
    for(unsigned int i = 0; i < rules.size(); i++){
        if(rule_matches_template(rules.at(i), templ)){
            return i;
        }
    }
    ///else error
    std::stringstream ss;
    ss << "Can't find rule matching template " << templ;
    std::cout << ss.str() << std::endl;
    /// debug
    //rmt_debug = true;
    //rule_matches_template(rules.at(0), templ);

    throw std::runtime_error(ss.str());
}


std::vector<int> template_to_rule_sequence(const std::vector<rule>& rules, const AST& templ){
    std::vector<int> res;
    /// if there are NT children, they need to be parsed first
    for(auto ch:templ.children){
        if(ch.children.size()){
            auto res2 = template_to_rule_sequence(rules, ch);
            res.insert(res.end(), res2.begin(), res2.end());
        }else{
            //res.push_back(RULE_IDX_SHIFT);
        }
    }
    /// finally, parse this node
    int idx = get_rule_index(rules, templ);
    res.push_back(idx);
    return res;
}
