#include "parser/parse_template.h"
#include "parser/span.h"
using rule = MiniParser::rule;
using AST = Parser::AST;

Parser::AST ast(std::string name, std::vector<Parser::AST> children){
    Parser::AST res;
    if(name.at(0) == '\\'){
        res.tok.text = name;
    }else{
        res.tok.type = name;
    }
    res.children = children;
    return res;
}


bool rule_matches_template(rule R, AST templ){
    Span<std::string> rule_span(R.previous);
    Span<AST> templ_span(templ.children);
    
    bool res = (templ == R.tok) && (templ_span == rule_span);
    
    return res;
}

int get_rule_index(const std::vector<rule>& rules, const AST& templ){
    for(unsigned int i = 0; i < rules.size(); i++){
        if(rule_matches_template(rules.at(i), templ)){
            return i;
        }
    }
    return -1;
}

// -1 means SHIFT next token onto stack
// 0..N means REDUCE using rule N
const int RULE_IDX_SHIFT = -1;

std::vector<int> template_to_rule_sequence(const std::vector<rule>& rules, const AST& templ){
    std::vector<int> res;
    /// if there are NT children, they need to be parsed first
    for(auto ch:templ.children){
        if(ch.children.size()){
            auto res2 = template_to_rule_sequence(rules, ch);
            res.insert(res.end(), res2.begin(), res2.end());
        }else{
            res.push_back(RULE_IDX_SHIFT);
        }
    }
    /// finally, parse this node
    int idx = get_rule_index(rules, templ);
    res.push_back(idx);
    return res;
}
