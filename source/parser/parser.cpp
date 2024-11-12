#include "parser/parser.h"
#include "parser/span.h"
#include <iostream>
#include <cassert>
// for debug output: 
#include "parser/ast_fixer.h"
#include "util/output.h"
//#include "parser/analyzer.h"
#include "parser/parse_template.h"

/// ---- MiniParser ---------------------------

using AST = MiniParser::AST;
using v_AST = MiniParser::v_AST;
using token = MiniParser::token;
using v_tokens = MiniParser::v_tokens;
using rule = MiniParser::rule;
using v_rules = MiniParser::v_rules;

void output_intermediate_ast(v_AST stack);

bool MiniParser::AST::operator==(std::string S) const{
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

bool MiniParser::does_rule_match(const v_AST &stack, AST lookahead, rule r){
    assert(stack.size());

    int sl = stack.size(); //stack len
    int rl = r.previous.size(); //rule len
    if(rl > sl){return false;}
    Span<std::string> rule_span(r.previous);
    Span_const<AST> stack_span(stack, sl-rl, rl);

    if((stack_span == rule_span) && (lookahead == r.lookahead)){
        //std::cout << "applying rule: " <<  concat(r.previous, " ") << " -> " << r.tok << std::endl;
        if(r.tok == "SHIFT"){should_shift = true;}
        return true;
    }else{
        return false;
    }
}

// bool MiniParser::try_apply_rule(v_AST &stack, AST lookahead, rule r){
//     assert(stack.size());
    
//     int sl = stack.size(); //stack len
//     int rl = r.previous.size(); //rule len
//     if(rl > sl){return false;}
//     Span<std::string> rule_span(r.previous);
//     Span<AST> stack_span(stack, sl-rl, rl);

//     if((stack_span == rule_span) && (lookahead == r.lookahead)){
//         //std::cout << "applying rule: " <<  concat(r.previous, " ") << " -> " << r.tok << std::endl;
//         if(r.tok == "SHIFT"){should_shift = true; return true;}

//         AST res;
//         res.tok.type = r.tok;
//         for(auto& a:stack_span){
//             res.children.push_back(a);
//         }
//         stack_span.erase();
//         stack.push_back(res);
//         return true;
//     }else{
//         return false;
//     }
// }


void MiniParser::apply_rule(v_AST &stack, rule r){
    assert(stack.size());
    
    int sl = stack.size(); //stack len
    int rl = r.previous.size(); //rule len
    Span<std::string> rule_span(r.previous);
    Span<AST> stack_span(stack, sl-rl, rl);

    if(r.tok == "SHIFT"){return;}

    AST res;
    res.tok.type = r.tok;
    for(auto& a:stack_span){
        res.children.push_back(a);
    }
    stack_span.erase();
    stack.push_back(res);
    
}

int n_applied_total = 0;

//void MiniParser::try_apply_rules(std::vector<AST> &stack, AST lookahead, const v_rules& rules){
//    should_shift = false;
//    bool retry = true;
//    /// keep trying all the rules until none of them apply
//    while(retry && !should_shift){
//        retry = false;
//        for(auto rule:rules){
//            if(try_apply_rule(stack, lookahead, rule)){
//                retry = true;
//                n_applied_total++; 
//                break; // go back to the beginning of the rule list to maintain priority
//            }
//        }
//    }
//}

std::string sal_ast_to_str(AST ast){
    if(ast.tok.type == "PUNCT"){
        return "\\"+ast.tok.text;
    }else return ast.tok.type;
}

std::string stack_and_lookahead_to_str(std::vector<std::string> prev, std::string lookahead){
    std::stringstream ss;
    for(auto tok:prev){
        ss << tok << " ";
    }
    ss << "." << " " << lookahead;
    return ss.str();
}

std::string stack_and_lookahead_to_str(const v_AST &stack, AST lookahead){
    std::vector<std::string> prev_toks;
    for(auto ast:stack){
        prev_toks.push_back(sal_ast_to_str(ast));
        if(prev_toks.size() > 10){prev_toks.erase(prev_toks.begin());}
    }
    auto lookahead_str = sal_ast_to_str(lookahead);
    return stack_and_lookahead_to_str(prev_toks, lookahead_str);
}

int MiniParser::get_next_rule_id(const v_AST &stack, AST lookahead, const v_rules& rules){
    std::cout << "get_next_rule(" << stack_and_lookahead_to_str(stack, lookahead) << "):" << std::endl;
    should_shift = false;
    for(unsigned int i = 0; i < rules.size(); i++){
        auto rule = rules.at(i);
        std::cout << "-is rule " << i << "? [" << stack_and_lookahead_to_str(rule.previous, rule.lookahead) << "] ";
        if(does_rule_match(stack, lookahead, rules.at(i))){
            std::cout << "yes ";
            if(should_shift){
                std::cout << "(shift)" << std::endl;
                return RULEID_SHIFT;
            }else{
                std::cout << i << std::endl;
                return i;
            }
        }
        std::cout << "no" << std::endl;
    }
    std::cout << "-no next rule " << RULEID_NONE << std::endl;
    return RULEID_NONE;
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

std::stringstream crs_prepare_msg(MiniParser::rseq ref_seq, std::vector<int> parse_seq){
    std::stringstream ss;
    ss << "MiniParser::ref_seq:\n";
    ss << "Ref ["; for(auto idx:*ref_seq){ss << idx << " ";} ss << "]\n";
    ss << "act ["; for(auto idx:parse_seq){ss << idx << " ";} ss << "]\n";
    return ss;
}

void check_reference_sequence(MiniParser::rseq ref_seq, int I, int rule_id, std::vector<int> parse_seq){
    if(!ref_seq){return;}
    if(I >= ref_seq->size()){
        auto ss = crs_prepare_msg(ref_seq, parse_seq);
        ss << "Error: current input longer than ref_seq";
        std::cerr << ss.str() << std::endl;   
        throw std::runtime_error(ss.str());
    }
    int ref_rule_id = ref_seq->at(I);
    if(rule_id != ref_rule_id){
        auto ss = crs_prepare_msg(ref_seq, parse_seq);
        ss << "Error: actual rule applied ("<<rule_id<<") doesn't match position " << I << " of ref_seq"; 
        std::cerr << ss.str() << std::endl;   
        throw std::runtime_error(ss.str());
    }
}

AST MiniParser::parse_stack(v_AST &stack, v_rules rules, rseq ref_seq){
    // algo: we add one token at a time and then try all the rules until there are none to apply
    n_applied_total = 0;

    assert(stack.size()); // we bug out when stack is empty (suspect span.h)
    AST tok_semicolon; tok_semicolon.tok.text = ";"; // for last token lookahead
    v_AST stack_out;

    std::vector<int> parse_seq; /// sequence of parse rule applications 
    int seq_I = 0;

    for(int I = 0; I < stack.size(); I++){
        AST node = stack[I];
        stack_out.push_back(node);
        AST lookahead = ((I+1) < stack.size()) ? stack[I+1] : tok_semicolon;
        //std::cout << "node ["; print_AST(node); std::cout << "], ["; print_AST(lookahead); std::cout << "]" << std::endl;
        //try_apply_rules(stack_out, lookahead, rules);
        bool retry = false;
        int rule_id = get_next_rule_id(stack_out, lookahead, rules);

        while(rule_id != RULEID_NONE){
            check_reference_sequence(ref_seq, seq_I, rule_id, parse_seq);
            seq_I++;
            if(rule_id == RULEID_SHIFT){break;}
            apply_rule(stack_out, rules.at(rule_id));
            parse_seq.push_back(rule_id);
            n_applied_total++;
            rule_id = get_next_rule_id(stack_out, lookahead, rules);
        }
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
            const AST &bad_ast = stack_out.back();//stack_out.at(1);
            const token &bad_tok = get_left_leaf(bad_ast).tok;
           
            int line = bad_tok.pos1.line;
            if(line != -1){
                ss << "at line " << line << ": ";
            }
            ss << " unexpected token [" << bad_tok << "]";
        }
        if(ref_seq){
            ss << "Reference sequence: [";
            for(auto idx : *ref_seq){ss << idx << " ";}
            ss << "]\n";
            ss << "Actual sequence:    [";
            for(auto idx : parse_seq){ss << idx << " ";}
            ss << "]\n";
        }else{ss << "(no ref_seq)";}
        std::cerr << ss.str() << std::endl;
        output_intermediate_ast(stack_out);
        throw std::runtime_error(ss.str());
    }
}

AST MiniParser::parse(v_tokens tokens, v_rules rules, rseq ref_seq){
    if(!tokens.size()){throw std::runtime_error("No tokens to parse");}
    auto stack = tokens_to_stack(tokens);
    AST res = parse_stack(stack, rules, ref_seq);
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

void output_intermediate_ast(v_AST stack){
    std::cout << "outputting intermediate AST..." << std::endl;
    AST ast;
    ast.tok.type = "intermediate stack";
    ast.children = stack;
    auto ast_pretty = ast_unroll_lists(ast);
    ast_pretty = ast_merge_singles(ast_pretty);
    calc_NT_positions(ast_pretty);
    auto ast_out = out.ast_to_json(ast_pretty);
    out.write_file("data_out/ast.json", ast_out);

}

/// having these pop up in text messes with VSC syntax highlighting.
#define L_CRL "{"
#define CRL_R "}"

void Parser::preproc_fold_curlies(v_AST &stack){
    unsigned int I = 0;

    /// consumes and erases everything from { to }, then spits out a node with the consumed tokens as children.
    /// recursively folds inner { }'s.
    std::function<AST(void)> fold_this_curly = [&]()->AST{
        AST a;
        a.tok.type = "{}";
        /// initial curly
        AST b = stack[I];
        assert(b == "\\" L_CRL);
        a.children.push_back(b);
        stack.erase(stack.begin()+I);

        while(I < stack.size()){
            AST b = stack[I];
            if(b == "\\" L_CRL){
                AST c = fold_this_curly();
                a.children.push_back(c);
            }else{
                a.children.push_back(b);
                stack.erase(stack.begin()+I);
                if(b == "\\" CRL_R){
                    return a;
                }
            }
        }
        throw std::runtime_error("mismatched curly-brace " L_CRL);
    };

    /// go over the stack, fold { }'s and ignore everything else.
    while(I < stack.size()){
        if(stack[I] == "\\" L_CRL){
            AST b = fold_this_curly();
            stack.insert(stack.begin()+I, b);
        }else if(stack[I] == "\\" CRL_R){
            AST a = stack[I];
            auto pos = a.tok.pos1;
            std::cout << "Parse error @ line " << pos.line << " (pos " << pos.char_idx << "):" << std::endl;
            std::cout << "mismatched closing curly-brace " CRL_R << std::endl;
            
            output_intermediate_ast(stack);
            std::cout << "outputting intermediate AST..." << std::endl;

            throw std::runtime_error("mismatched curly-brace " CRL_R);
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

AST Parser::process(v_tokens tokens, std::optional<AST> parse_template){
    MiniParser MP;
    AST res;
    bool do_preproc = true;

    MiniParser::rseq ref_seq;
    if(parse_template){
        ref_seq = template_to_rule_sequence(rules, *parse_template);
    }

    if(do_preproc){
        auto prep = preprocess(tokens);
        //std::cout << "after preprocess:" << std::endl;
        //MiniParser::print_stack(prep);
        res = MP.parse_stack(prep, rules, ref_seq);
    }else{
        res = MP.parse(tokens, rules, ref_seq);
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
        assert(ast.children.front().tok.text == L_CRL);
        assert(ast.children.back().tok.text == CRL_R);
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


