#include "parser/parser.h"
#include "parser/span.h"
#include <iostream>
#include <cassert>
// for debug output: 
#include "parser/ast_fixer.h"
#include "util/output.h"
//#include "parser/analyzer.h"
#include "parser/parse_template.h"
#include <functional>
/// ---- MiniParser ---------------------------

using AST = MiniParser::AST;
using v_AST = MiniParser::v_AST;
using token = MiniParser::token;
using v_tokens = MiniParser::v_tokens;
using rule = MiniParser::rule;
using v_rules = MiniParser::v_rules;

void output_intermediate_ast(v_AST stack);
std::string output_reconstructed_source(AST ast);
std::string output_reconstructed_source(v_AST stack);

bool MiniParser::AST::operator==(std::string S) const{
    if(S == ""){return false;}
    if(S == "*"){return true;}
    if(tok.type == "*"){return true;}
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
    //std::cout << "get_next_rule(" << stack_and_lookahead_to_str(stack, lookahead) << "):" << std::endl;
    should_shift = false;
    for(unsigned int i = 0; i < rules.size(); i++){
        auto rule = rules.at(i);
        //std::cout << "-is rule " << i << "? [" << stack_and_lookahead_to_str(rule.previous, rule.lookahead) << "] ";
        if(does_rule_match(stack, lookahead, rules.at(i))){
            //std::cout << "yes ";
            if(should_shift){
                //std::cout << "(shift)" << std::endl;
                return RULEID_SHIFT;
            }else{
                //std::cout << i << std::endl;
                return i;
            }
        }
        //std::cout << "no" << std::endl;
    }
    //std::cout << "-no next rule " << RULEID_NONE << std::endl;
    return RULEID_NONE;
}

/// -------------------------------- BEGIN CPFR Algo (count possible future rules (to complete the program)) ----------------

typedef unsigned int uint;

/// reverse iterator to paired ranges 

enum eContAccessType{ECAT_F, ECAT_CF, ECAT_R, ECAT_CR};

template<typename T, eContAccessType cat> struct cont_types{
    using cont = T;
    using val = T::value_type;
    using iter = T::iterator;
    static iter begin(cont& container){assert(!"abstract called"); return container.begin();}
    static iter end(cont& container){assert(!"abstract called"); return container.end();}
};

template<typename T> struct cont_types<T, ECAT_F>{
    using cont = T;
    using val = T::value_type;
    using iter = T::iterator;
    static iter begin(cont& container){return container.begin();}
    static iter end(cont& container){return container.end();}
};

template<typename T> struct cont_types<T, ECAT_CF>{
    using cont = const T;
    using val = const T::value_type;
    using iter = T::const_iterator;
    static iter begin(cont& container){return container.cbegin();}
    static iter end(cont& container){return container.cend();}
};

template<typename T> struct cont_types<T, ECAT_R>{
    using cont = T;
    using val = T::value_type;
    using iter = T::reverse_iterator;
    static iter begin(cont& container){return container.rbegin();}
    static iter end(cont& container){return container.rend();}
};

template<typename T> struct cont_types<T, ECAT_CR>{
    using cont = const T;
    using val = const T::value_type;
    using iter = T::const_reverse_iterator;
    static iter begin(cont& container){return container.crbegin();}
    static iter end(cont& container){return container.crend();}
};

template<typename CTa, typename CTb> struct zip_unpack{
    CTa::val &a;
    CTb::val &b;
    int idx;
};

template<typename CTa, typename CTb> struct zip_iter{
    CTa::iter a;
    CTb::iter b;
    int idx;

    zip_iter &operator++(){
        ++a; ++b; --idx;
        return *this;//rzip_iter{++a,++b,--idx};
    }
    zip_iter operator++(int){
        return zip_iter{a++,b++,idx--};
    }
    bool operator!=(const zip_iter& other) const{
        return !(*this == other);
    }
    bool operator==(const zip_iter& other) const{
        return (a == other.a) && (b == other.b) && (idx == other.idx);
    }
    zip_unpack<CTa,CTb> operator*(){
        return zip_unpack<CTa, CTb>{*a,*b,idx};
    }
};

template<typename CTa, typename CTb> struct zip_holder{
    CTa::cont &a;
    CTb::cont &b;
    zip_iter<CTa,CTb> begin(){
        return zip_iter<CTa, CTb>{CTa::begin(a), CTb::begin(b), a.size()-1};
    }
    zip_iter<CTa,CTb> end(){
        return zip_iter<CTa, CTb>{CTa::end(a), CTb::end(b), -1}; // idx[0] = ok, idx[-1] - one-past-the-end = end
    }
};


template<typename Ta, typename Tb> zip_holder<cont_types<Ta, ECAT_F>,  cont_types<Tb, ECAT_F>>  zip(      Ta &a,       Tb &b)
{assert(a.size() == b.size());return zip_holder<cont_types<Ta, ECAT_F>,  cont_types<Tb, ECAT_F>>{a,b};}
template<typename Ta, typename Tb> zip_holder<cont_types<Ta, ECAT_CF>, cont_types<Tb, ECAT_F>>  zip(const Ta &a,       Tb &b)
{assert(a.size() == b.size());return zip_holder<cont_types<Ta, ECAT_CF>, cont_types<Tb, ECAT_F>>{a,b};}
template<typename Ta, typename Tb> zip_holder<cont_types<Ta, ECAT_F>,  cont_types<Tb, ECAT_CF>> zip(      Ta &a, const Tb &b)
{assert(a.size() == b.size());return zip_holder<cont_types<Ta, ECAT_F>,  cont_types<Tb, ECAT_CF>>{a,b};}
template<typename Ta, typename Tb> zip_holder<cont_types<Ta, ECAT_CF>, cont_types<Tb, ECAT_CF>> zip(const Ta &a, const Tb &b)
{assert(a.size() == b.size());return zip_holder<cont_types<Ta, ECAT_CF>, cont_types<Tb, ECAT_CF>>{a,b};}

template<typename Ta, typename Tb> zip_holder<cont_types<Ta, ECAT_R>,  cont_types<Tb, ECAT_R>>  rzip(      Ta &a,       Tb &b)
{assert(a.size() == b.size());return zip_holder<cont_types<Ta, ECAT_R>,  cont_types<Tb, ECAT_R>>{a,b};}
template<typename Ta, typename Tb> zip_holder<cont_types<Ta, ECAT_CR>, cont_types<Tb, ECAT_R>>  rzip(const Ta &a,       Tb &b)
{assert(a.size() == b.size());return zip_holder<cont_types<Ta, ECAT_CR>, cont_types<Tb, ECAT_R>>{a,b};}
template<typename Ta, typename Tb> zip_holder<cont_types<Ta, ECAT_R>,  cont_types<Tb, ECAT_CR>> rzip(      Ta &a, const Tb &b)
{assert(a.size() == b.size());return zip_holder<cont_types<Ta, ECAT_R>,  cont_types<Tb, ECAT_CR>>{a,b};}
template<typename Ta, typename Tb> zip_holder<cont_types<Ta, ECAT_CR>, cont_types<Tb, ECAT_CR>> rzip(const Ta &a, const Tb &b)
{assert(a.size() == b.size());return zip_holder<cont_types<Ta, ECAT_CR>, cont_types<Tb, ECAT_CR>>{a,b};}


/*
template<typename Ta, typename Tb> struct rzip_unpack{
    Ta::value_type &a;
    Tb::value_type &b;
    int idx;
};

template<typename Ta, typename Tb> struct rzip_iter{
    Ta::reverse_iterator a;
    Tb::reverse_iterator b;
    int idx;

    rzip_iter &operator++(){
        ++a; ++b; --idx;
        return *this;//rzip_iter{++a,++b,--idx};
    }
    rzip_iter operator++(int){
        return rzip_iter{a++,b++,idx--};
    }
    bool operator!=(const rzip_iter& other) const{
        return !(*this == other);
    }
    bool operator==(const rzip_iter& other) const{
        return (a == other.a) && (b == other.b) && (idx == other.idx);
    }
    rzip_unpack<Ta,Tb> operator*(){
        return rzip_unpack{*a,*b,idx};
    }
};

template<typename Ta, typename Tb> struct rzip_holder{
    Ta &a;
    Tb &b;
    rzip_iter<Ta,Tb> begin(){
        return rzip_iter{a.rbegin(), b.rbegin(), a.size()-1};
    }
    rzip_iter<Ta,Tb> end(){
        return rzip_iter{a.rend(), b.rend(), 0};
    }
};

template<typename Ta, typename Tb> rzip_holder<Ta, Tb> rzip(Ta &a, Tb &b){
    assert(a.size() == b.size());
    return rzip_holder{a,b};
}
*/
///-----------------------------
/*
template<typename Ta, typename Tb> struct crzip_unpack{
    const Ta::value_type &a;
    const Tb::value_type &b;
    int idx;
    //crzip_unpack(
    //    const Ta::value_type &a,
    //    const Tb::value_type &b,
    //    int idx
    //):a(a),b(b),idx(idx){}
};

template<typename Ta, typename Tb> struct crzip_iter{
    Ta::const_reverse_iterator a;
    Tb::const_reverse_iterator b;
    int idx;

    crzip_iter &operator++(){
        ++a; ++b; --idx;
        return *this;//crzip_iter{++a,++b,--idx};
    }
    crzip_iter operator++(int){
        return crzip_iter{a++,b++,idx--};
    }
    bool operator!=(const crzip_iter& other) const{
        return !(*this == other);
    }
    bool operator==(const crzip_iter& other) const{
        return (a == other.a) && (b == other.b) && (idx == other.idx);
    }
    crzip_unpack<Ta,Tb> operator*(){
        return crzip_unpack<Ta,Tb>{*a,*b,idx};
    }
};

template<typename Ta, typename Tb> struct crzip_holder{
    const Ta &a;
    const Tb &b;
    crzip_iter<Ta,Tb> begin(){
        return crzip_iter<Ta,Tb>{a.crbegin(), b.crbegin(), a.size()-1};
    }
    crzip_iter<Ta,Tb> end(){
        return crzip_iter<Ta,Tb>{a.crend(), b.crend(), 0};
    }
};
*/
//template<typename Ta, typename Tb> crzip_holder<Ta, Tb> rzip(const Ta &a, const Tb &b){
//    assert(a.size() == b.size());
//    return crzip_holder<Ta,Tb>{a,b};
//}
///------------------
///---------------- half-const implementation
//template<typename Ta, typename Tb> hcrzip_holder<Ta, Tb> rzip(const Ta &a, Tb &b){
//    assert(a.size() == b.size());
//    return hcrzip_holder<Ta,Tb>{a,b};
//}
///------------------


int max(int A, int B){return (A > B)? A : B;}
int min(int A, int B){return (A < B)? A : B;}

uint longest_rule_length(const v_rules& rules){
    uint len = 0;
    for(auto &rule:rules){
        len = max(rule.previous.size(), len);
    }
    return len;
}

//std::string get_tok_name(token tok){
//    return (tok.type == "")? "\\"+tok.text : tok.type;
//}

AST tok_to_ast(token tok){
    AST res;
    res.tok = tok;
    return res;
}

std::string ssprint_with_delim_helper(std::vector<std::string> stuff, std::string delim){
    std::stringstream ss;

    auto begin = stuff.begin();
    auto end = stuff.end();
    for(auto I = begin; I != end; I++){
        ss << *I;
        if((I+1) != end){ss << delim;}
    }
    return ss.str();
}

template<typename T>
std::string ssprint(T obj){std::stringstream ss; ss << obj; return ss.str();}

template<typename T>
std::string ssprint_with_delim(T vec, std::string delim){
    std::vector<std::string> stuff; for(auto &thing:vec){stuff.push_back(ssprint(thing));}
    return ssprint_with_delim_helper(stuff, delim);
}

void dbg_print_stack(const v_AST &stack){
    std::cout << "stack [" << ssprint_with_delim(stack, ", ") << "]" << std::endl;
}

void dbg_print_one_set(const std::set<token> &ts){
    std::cout << "set [" << ssprint_with_delim(ts, ", ") << "]" << std::endl;
}

void dbg_print_all_sets(const std::vector<std::set<token>> &sets){
    std::cout << "(" << sets.size() << " sets){" << std::endl;
    for(int i = 0; i < sets.size(); i++){
        std::cout << "\t" << i << ": "; dbg_print_one_set(sets[i]);
    }
    std::cout << "}" << std::endl; 
}



v_AST cpfr_prepare_stack(const v_AST &stack, AST sym, uint idx, uint max_rule_len){
    v_AST res;                         /// the new stack consists of:
    for(uint i = 0; i < idx; i++){
        res.push_back(stack.at(i));    /// ... original stack up to idx
    }   
    res.push_back(sym);                /// ... the requested symbol
    while(max_rule_len--){
        res.push_back(ast("*"));       /// ... and a number of "any" tokens
    }                 
    return res;
}

void cpfr_attempt_stack(const v_AST &stack, uint idx, const v_rules& rules, std::vector<std::set<token>> &sets_out){
    //std::cout << "   attempt stack(idx = " << idx << ")";
    int max_rule_size = longest_rule_length(rules);
    int max_init_offset = min(idx, max(max_rule_size, stack.size()));
    //std::cout << ", offset from " << max_init_offset << "to 0 incl." << std::endl;
    /// add reductions to this set, prev set, prev prev etc.
    for(int offset = max_init_offset; offset >= 0; offset--){
        //std::cout << "    try offset = " << offset << std::endl;
        int i = 0;
        for(auto &rule:rules){                                                      /// check all 50+ rules
            auto rule_size = rule.previous.size();
            int init_offset = min(idx, max(rule_size, stack.size()));
            if(offset > init_offset){
                //std::cout << "rule "<<i++<<"is too short" << std::endl; 
                continue;}
            //std::cout << "    try rule " << i++ << ":["<<rule<<"]\t";
            int start_idx = idx - offset;
            Span_const<std::string> rule_span(rule.previous);
            Span_const<AST> stack_span(stack, start_idx, rule_size);
            //std::cout << std::endl;
            //std::cout << "     compare rule_span ["<< rule_span<<"]" << std::endl;
            //std::cout << "     vs     stack_span ["<<stack_span<<"]" << std::endl;
            if(stack_span==rule_span){//if(rule_span == stack_span){                                        /// if a shifted rule matches, the result of the rule (reduction)
                auto res = token(rule.tok);
                if(res.type == "SHIFT"){ 
                    /// if we shift, then: 
                    /// - there is an "any token" after this one
                    /// -- but we are already assuming that
                    /// - other rules do not match from this offset (move one index forward)
                    //std::cout << "<match, SHIFTing> ";
                    continue;
                }
                //std::cout << "<match, adding res ["<<res<<"]> ";
                sets_out.at(start_idx).insert(res);                     /// is added to the replacement set at the position to which we shifted.
            }
            //std::cout << std::endl;
        }
    }
}

void cpfr_attempt_sym(const v_AST &stack, const v_rules &rules, std::vector<std::set<token>> &sets, AST sym, uint idx, uint rule_len){
    auto as_if_stack = cpfr_prepare_stack(stack, sym, idx, rule_len);
    //std::cout << "  attempt sym: new "; dbg_print_stack(as_if_stack);
    cpfr_attempt_stack(as_if_stack, idx, rules, sets);
}

/// calls the function with every element of a set as argument,
/// given that the function may add or remove elements in the middle of execution.
/// stops running when the set runs out of unvisited elements.
template<typename T> void run_for_old_and_new_set_elements(std::set<T> &set, std::function<void(T)> foo){
    std::set<T> visited;
    while(true){
        auto I = std::find_if(set.begin(), set.end(), [&](const T& val)->bool{return !visited.count(val);});
        if(I == set.end()){break;}
        foo(*I);
        visited.insert(*I);
    }
}


typedef std::set<token> tokset;
token CPFR_target = token{"program"};
std::vector<tokset> CPFR_last_sets;
/// checks if any of the future rules can possibly reduce the AST to "start" ("program") token.
/// true: there is some possible future input by the user such that the source code parses correctly
/// false: the parser is jammed by an unexpected token
bool count_possible_future_rules(const v_AST &stack, /*AST lookahead,*/ const v_rules& rules){ // lookahead symbol is unused
    const uint rule_len = longest_rule_length(rules);
    std::vector<tokset> sets(stack.size());
    //std::cout << "CPFR, begin. rule_len = " << rule_len << ";" << std::endl;
    //dbg_print_stack(stack);
    //dbg_print_all_sets(sets);
    std::cout << "CPFR" << std::endl;
    for(auto [sym,set,idx]:rzip(stack, sets)){
        std::cout << " cpfr idx(" << idx << "):"<<sym.tok<<std::endl;
        //std::cout << " ---- loop ---- idx " << idx << std::endl;
        //std::cout << "  CPFR add sym " << sym << std::endl;
        if(idx != 0){               /// do not add the initial stack element to the "visited" set
            set.insert(sym.tok);    /// as we only want the first elements that are obtained by reduction (meaning nothing comes after them)
        }
        //dbg_print_all_sets(sets);
        std::function<void(token)> lmb_attempt_sym = [&](token arg){cpfr_attempt_sym(stack, rules, sets, tok_to_ast(arg), idx, rule_len);};
        
        lmb_attempt_sym(sym.tok);
        run_for_old_and_new_set_elements(set, lmb_attempt_sym);
        //std::cout << " ---- endloop --- idx " << idx << std::endl;
    }
    //std::cout << "loop done" << std::endl;
    //dbg_print_all_sets(sets);
    CPFR_last_sets = sets;
    return sets.front().count(CPFR_target);   
}
    /// CPFR ponder #2 (06.02.2025)
        /// - when SHIFT encountered, we can essentially have any terminal as the next symbol
        /// - we might have to do breadth-first search over all possible programs
        /// -- currently we are doing depth-first search (rly?) which may never terminate (expr. A+A+A+A...)
        /// -- mitigation: re-order rules to "generate" shortest possible programs first
        /// -- can we avoid that? prolly not in CPFR, maybe first-follow sets do something or not idk.
        /// - Need the ability to select target top-level symbol - not all tests are "complete programs"
        /// -- some are expressions or control structures e.g. unit-tests.

    /// count_possible_future_rules ponder:
        /// not sure if this is even solvable
        /// A B C D - good
        ///  A B... A B E... A B E F... A B C... A B C K... A B C K G... A B C D ... A E.... X
        /// A C D B - bad
        ///  A C... A C D... A C D B... A C K ... no rules.
        /// maybe build a "reachability tree" that contains all (unexpanded) possible completions
        ///  and then prune branches that can't be
        /// first of all, why don't we have Rules in a tree or a graph? we should grab that.
        /// it's going to be cyclic though.
        
        /// Here is an algorythm to calculate whether the current stack is valid (has a possible future).
        /// We only need to run it when we encounter an error - we go back to our stack and calculate.
        ///
        /// 1. any symbol can be added after the current stack (by adding all it's components)
        ///         ... except some previous symbols might force a shift...
        /// 2. a subset N of all rules can consume stack[1]
        ///     of these rules, n rules can be applied as-is by adding the appropriate symbols (i.e. simply possible future rules, SPFR)
        ///     other M rules conflict with the next few symbols
        /// 3. For the next symbol, stack[I+1], another subset of rules can consume them, producing a set of possible tokens.
        /// actually, lets start from the end.
        /// 3b. The last symbol, stack[E-1], can be reduced (either alone or with other previous symbols) if the correct future symbols are added.
        ///     this reducting replaces the last symbol (and possible earlier symbols) with a different symbol.
        ///     For each position, this creates a set of possible replacement sybmols.
        /// 4. We treat each "replacement" symbol as though it becomes the new tail (assuming the symbols after it disappear),
        ///     because the new symbol can only appear during a reduction.
        ///     therefore each symbol is a possible future tail. We therefore deal with multiple possible future stacks,
        ///     some of them might be shorter.
        /// 5. We move to previous symbol, stack[E-2], and for it and all the symbols in the set of replacements so far, repeat (3b)
        ///     to populate the previous sets with possible symbols.
        /// 6. Repeat until we got to the replacements of the initial symbol (stack[0]). If the set is non-empty, that means stack[1] can still be reduced.
        ///     and the current stack is still valid and has a future.
        /// 7. If the initial symbol has an empty replacement set, and there is still un-reduced input, that means the last added symbol is a syntax error.


/// --------------------------------------- END CPFR Algo ---------------------------------------------

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

    if(ref_seq && ref_seq->size()){
        auto last_rule_id = ref_seq->back();
        auto &last_rule = rules[last_rule_id];
        CPFR_target = token{last_rule.tok};
        std::cout << "Parse_stack: CPFR target token is " << CPFR_target << std::endl;
    }

    int last_rule_id = 0;
    int last_cpfr = 0;
    for(int I = 0; I < stack.size(); I++){
        AST node = stack[I];
        stack_out.push_back(node);
        AST lookahead = ((I+1) < stack.size()) ? stack[I+1] : tok_semicolon;
        int n_futures = count_possible_future_rules(stack_out, rules);
        //std::cout << "node ["; print_AST(node); std::cout << "], ["; print_AST(lookahead); std::cout << "]" << std::endl;
        //try_apply_rules(stack_out, lookahead, rules);
        bool retry = false;
        int rule_id = get_next_rule_id(stack_out, lookahead, rules);
        last_rule_id = rule_id;
        last_cpfr = n_futures;
        if((rule_id != RULEID_NONE) && (n_futures == 0)){
            std::stringstream ss;
            ss << "internal error: CPFR failed, but get_next_rule_id()="<<rule_id;
            std::cerr << ss.str() << std::endl;
            n_futures = count_possible_future_rules(stack_out, rules);
            throw std::runtime_error(ss.str());
        }
        if((rule_id == RULEID_NONE) && (n_futures == 0)){
            std::stringstream ss;
            ss << "internal error: CPFR passed, but get_next_rule_id()="<<rule_id;
            std::cerr << ss.str() << std::endl;
            n_futures = count_possible_future_rules(stack_out, rules);
            throw std::runtime_error(ss.str());
            
        }
        if((rule_id == RULEID_NONE) && (n_futures == 0)){
            std::cout << "CPFR and get_next_rule_id in agreement"<<std::endl;
        }
        if(stack_out.size() >= 2 && n_futures == 0){
            std::cerr << "Error: n_futures == 0" << std::endl;
            goto lbl_syntax_error;
        }
        while(rule_id != RULEID_NONE){
            if(rule_id == RULEID_SHIFT){break;}
            check_reference_sequence(ref_seq, seq_I, rule_id, parse_seq);
            seq_I++;
            apply_rule(stack_out, rules.at(rule_id));
            parse_seq.push_back(rule_id);
            n_applied_total++;
            rule_id = get_next_rule_id(stack_out, lookahead, rules);
        }
        /// all rules applied for tokens seen so far, continue to the rest of the tokens
    }
    /// end of input

    std::cout << "final CPFR: " << last_cpfr << ", rule_id: " << last_rule_id << std::endl;
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
    else{ 
        std::cout << "\n\n";
        std::cout << "End of input but something is wrong." << std::endl;
        if(last_cpfr == 1){
            std::cout << "CPFR says additional input is needed to complete [" << CPFR_target << "]" << std::endl;
            std::cout << "CPFR sets:" << std::endl;
            dbg_print_all_sets(CPFR_last_sets);
        }

        lbl_syntax_error:
        //if(stack2.size() != 1)
        /// that's bad - syntax error
        //std::cout<< "MiniParser: syntax error." << std::endl;
        //print_stack(stack_out);
        
        //throw std::runtime_error("Unparsed tokens remain after trying all rules.");
        std::stringstream ss;
        ss << "Syntax error: ";
        if(stack_out.size() > 2){
            /// the second token is probably the
            /// cause of error
        //    const AST &bad_ast = stack_out.back();//stack_out.at(1);
        //    const token &bad_tok = get_left_leaf(bad_ast).tok;
        //   
        //    int line = bad_tok.pos1.line;
        //    if(line != -1){
        //        ss << "at line " << line << ": ";
        //    }
        //    ss << " unexpected token [" << bad_tok << "]";

            auto get_pos = [](const AST& bad_ast)->int{
                const token &bad_tok = get_left_leaf(bad_ast).tok;
                int line = bad_tok.pos1.line;
                return line;
            };

            ss << "printing parser stack:\n";
            for(auto i = 0; i < stack_out.size(); i++){
                auto &ast = stack_out.at(i);
                ss << "[stack " << i << " @ line " << get_pos(ast) << "]: " << ast << "\n";
            }

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
        std::cerr << "\nreconstructed source:\n"<<std::endl;
        std::cerr << output_reconstructed_source(stack_out);
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

std::string output_reconstructed_source(AST ast){
    std::stringstream ss;
    ss << ast.tok.text;
    for(auto &ch:ast.children){
        ss << output_reconstructed_source(ch);
    }
    ss << " ";
    return ss.str();
}

std::string output_reconstructed_source(v_AST stack){
    std::stringstream ss;
    for(auto &ast:stack){
        ss << output_reconstructed_source(ast);
    }
    return ss.str();
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
    if(tok.text.empty()){
        return stream << tok.type;
    }else{
        return stream << "[" << tok.text << "]:" << tok.type;
    }
}

std::ostream& operator<<(std::ostream& stream, MiniParser::AST ast){
    stream << ast.tok;
    if(ast.children.size()){
        stream << "(";
        for(auto I = ast.children.begin(); I != ast.children.end(); I++){
            auto &ch = *I; bool last = ((I+1) == ast.children.end());
            stream << ch;
            if(!last){stream << ",";}
        }
        stream << ")";
    }
    return stream;
}

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

// std::ostream& operator<<(std::ostream& stream, MiniParser::rule rule){
//     stream << "[";
//     for(auto I = rule.previous.begin(); I != rule.previous.end(); I++){
//         auto sym = *I; bool last = ((I+1) == rule.previous.end());
//         stream << sym;
//         if(!last){stream << " ";} 
//     }
//     stream << ". " << rule.lookahead << " -> " << rule.tok << "]";
//     return stream;
// }

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


