#include "parser/parser.h"
#include "parser/span.h"
#include <iostream>
#include <cassert>
//#include "parser/analyzer.h"
/// ---- MiniParser ---------------------------

using AST = MiniParser::AST;
using v_AST = MiniParser::v_AST;
using token = MiniParser::token;
using v_tokens = MiniParser::v_tokens;
using rule = MiniParser::rule;
using v_rules = MiniParser::v_rules;

bool MiniParser::AST::operator==(std::string S){
    if(S == ""){return false;}
    if(S == "*"){return true;}
    if(S[0] == '\\'){
        return (tok.text == S.substr(1)) && (children.size() == 0);
    }else{
        return (tok.type == S);
    }
}

void MiniParser::print_tok(const token tok){
    std::cout << "[" << tok.text << "]:"<<tok.type;
}

void MiniParser::print_AST(AST ast, int indent){
    for(int i = 0; i < indent; i++){std::cout << " ";}
    //if(ast.tok.text != ""){std::cout << "[" << ast.tok.text << "]";}
    //else{std::cout << ast.tok.type;}
    print_tok(ast.tok);

    if(ast.children.size()){
        std::cout << std::endl;
        for(auto ch:ast.children){
            print_AST(ch, indent+1);
        }
        std::cout << std::endl;
    }
}

void MiniParser::print_stack(const v_AST& ast){
    int i = 0;
    for(auto a:ast){
        std::cout << i++ << ": { ";
        print_AST(a);
        std::cout << " }" << std::endl;
    }
}

std::string concat(std::vector<std::string> v, std::string delim){
    std::string S;
    bool first = true;
    for(auto s:v){
        if(first){first = false;}else{S += delim;}
        S += s;
    }
    return S;
}

bool should_shift = false; /// waah, global, idk.

bool MiniParser::try_apply_rule(v_AST &stack, AST lookahead, rule r){
    assert(stack.size());
    
    int sl = stack.size(); //stack len
    int rl = r.previous.size(); //rule len
    if(rl > sl){return false;}
    Span<std::string> rule_span(r.previous);
    Span<AST> stack_span(stack, sl-rl, rl);

    if((stack_span == rule_span) && (lookahead == r.lookahead)){
        //std::cout << "applying rule: " <<  concat(r.previous, " ") << " -> " << r.tok << std::endl;
        if(r.tok == "SHIFT"){should_shift = true; return true;}

        AST res;
        res.tok.type = r.tok;
        for(auto& a:stack_span){
            res.children.push_back(a);
        }
        stack_span.erase();
        stack.push_back(res);
        return true;
    }else{
        return false;
    }
}


int n_applied_total = 0;

void MiniParser::try_apply_rules(std::vector<AST> &stack, AST lookahead, const v_rules& rules){
    should_shift = false;
    bool retry = true;
    /// keep trying all the rules until none of them apply
    while(retry && !should_shift){
        retry = false;
        for(auto rule:rules){
            if(try_apply_rule(stack, lookahead, rule)){
                retry = true;
                n_applied_total++; 
                break; // go back to the beginning of the rule list to maintain priority
            }
        }
    }
}

v_AST MiniParser::tokens_to_stack(v_tokens tokens){
    std::vector<AST> stack;
    for(auto tok:tokens){
        AST a;
        a.tok = tok;
        stack.push_back(a);
    }
    return stack;
}

const AST &get_left_leaf(const AST& ast){
    const AST *t = &ast;
    while(t->children.size()){
        t = &(t->children.front());
    }
    return *t;
}

const AST &get_right_leaf(const AST& ast){
    const AST *t = &ast;
    while(t->children.size()){
        t = &(t->children.back());
    }
    return *t;
}

AST MiniParser::parse_stack(v_AST &stack, v_rules rules){
    // algo: we add one token at a time and then try all the rules until there are none to apply
    n_applied_total = 0;

    assert(stack.size()); // we bug out when stack is empty (suspect span.h)
    AST tok_semicolon; tok_semicolon.tok.text = ";"; // for last token lookahead
    v_AST stack_out;

    for(int I = 0; I < stack.size(); I++){
        AST node = stack[I];
        stack_out.push_back(node);
        AST lookahead = ((I+1) < stack.size()) ? stack[I+1] : tok_semicolon;
        //std::cout << "node ["; print_AST(node); std::cout << "], ["; print_AST(lookahead); std::cout << "]" << std::endl;
        try_apply_rules(stack_out, lookahead, rules);
    }

    //std::cout << "MiniParser::parse_stack applied " << n_applied_total << " rules total." << std::endl;

    if(stack_out.size() == 0){
        /// that's bad - nothing to parse?
        throw std::runtime_error("stack empty");
    }
    if(stack_out.size() == 1){
        /// all good!
        AST res = stack_out.at(0);
        return res;
    }
    else{ //if(stack2.size() != 1)
        /// that's bad - syntax error
        //std::cout<< "MiniParser: syntax error." << std::endl;
        //print_stack(stack_out);
        
        //throw std::runtime_error("Unparsed tokens remain after trying all rules.");
        std::stringstream ss;
        ss << "Syntax error: ";
        if(stack_out.size() > 2){
            /// the second token is probably the
            /// cause of error
            const AST &bad_ast = stack_out.at(1);
            const token &bad_tok = get_left_leaf(bad_ast).tok;
           
            int line = bad_tok.pos1.line;
            if(line != -1){
                ss << "at line " << line << ": ";
            }
            ss << " unexpected token [" << bad_tok << "]";
        }
        std::cerr << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

AST MiniParser::parse(v_tokens tokens, v_rules rules){
    if(!tokens.size()){throw std::runtime_error("No tokens to parse");}
    auto stack = tokens_to_stack(tokens);
    AST res = parse_stack(stack, rules);
    return res;
}

/// ----------- Parser ---------------------------------

void Parser::preproc_unquote_strings(v_AST &stack){
    /// finds all STRING tokens and removes " from start and end of string
    for(auto &ast : stack){
        if(ast.tok.type == "STRING"){
            std::string text = ast.tok.text;
            if(text[0] == '\"'){text = text.substr(1);} /// remove first char
            if(text.back() == '\"'){text.pop_back();}
            ast.tok.text = text;
        }
    }
}

void Parser::preproc_fold_curlies(v_AST &stack){
    unsigned int I = 0;

    /// consumes and erases everything from { to }, then spits out a node with the consumed tokens as children.
    /// recursively folds inner { }'s.
    std::function<AST(void)> fold_this_curly = [&]()->AST{
        AST a;
        a.tok.type = "{}";
        /// initial curly
        AST b = stack[I];
        assert(b == "\\{");
        a.children.push_back(b);
        stack.erase(stack.begin()+I);

        while(I < stack.size()){
            AST b = stack[I];
            if(b == "\\{"){
                AST c = fold_this_curly();
                a.children.push_back(c);
            }else{
                a.children.push_back(b);
                stack.erase(stack.begin()+I);
                if(b == "\\}"){
                    return a;
                }
            }
        }
        throw std::runtime_error("mismatched curly-brace {");
    };

    /// go over the stack, fold { }'s and ignore everything else.
    while(I < stack.size()){
        if(stack[I] == "\\{"){
            AST b = fold_this_curly();
            stack.insert(stack.begin()+I, b);
        }else if(stack[I] == "\\}"){
            throw std::runtime_error("mismatched curly-brace }");
        }else{
            I++;
        }
    }

    /// OK we folded all {} and next we want to group commands i.e. from ; to ; is a command.
}

v_AST Parser::preprocess(v_tokens tokens){
    v_AST stack = MiniParser::tokens_to_stack(tokens);
    preproc_unquote_strings(stack);
    preproc_fold_curlies(stack);
    return stack;
}

AST Parser::process(v_tokens tokens){
    MiniParser MP;
    AST res;
    bool do_preproc = true;
    if(do_preproc){
        auto prep = preprocess(tokens);
        //std::cout << "after preprocess:" << std::endl;
        //MiniParser::print_stack(prep);
        res = MP.parse_stack(prep, rules);
    }else{
        res = MP.parse(tokens, rules);
    }

    parse_braces(res);

    //MiniParser::print_AST(res);
    //std::cout << "Parse done" << std::endl;

    return res;
}

std::ostream& operator<<(std::ostream& stream, Tokenizer::token tok){
    return stream << "[" << tok.text << "]:" << tok.type;
}

void Parser::parse_braces(AST &ast){
    if(ast.tok.type == "{}"){
        // make sure the first and last tokens are { } and erase them
        assert(ast.children.front().tok.text == "{");
        assert(ast.children.back().tok.text == "}");
        ast.children.pop_back();
        ast.children.erase(ast.children.begin());
        // ok now parse normally
        MiniParser MP;
        AST ast2 = MP.parse_stack(ast.children, rules);
        ast.children = {ast2};
    }
    // recurse to find any other {}s
    for(auto &ch:ast.children){parse_braces(ch);}
}