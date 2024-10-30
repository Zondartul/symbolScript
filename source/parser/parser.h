#pragma once
#include "parser/tokenizer.h"

class MiniParser{
    public:
    /// todo: wanna make it depend on MiniTokenizer 
    /// instead of Tokenizer, but still 
    /// somehow preserve the extra data.
    using token = Tokenizer::token; 
    using v_tokens = std::vector<token>;
    struct AST{
        token tok;
        std::vector<AST> children;
        bool operator==(std::string S);
    };
    using v_AST = std::vector<AST>;
    struct rule{
        std::vector<std::string> previous;
        std::string lookahead;
        std::string tok;
    };
    using v_rules = std::vector<rule>;

    static void print_AST(AST ast, int indent = 0);
    static void print_stack(const v_AST& stack);
    static void print_tok(const token tok);
    static v_AST tokens_to_stack(v_tokens tokens);
    static AST parse_stack(v_AST& stack, v_rules);
    static AST parse(v_tokens, v_rules);
    static bool try_apply_rule(v_AST &stack, AST lookahead, rule r);
    static void try_apply_rules(std::vector<AST> &stack, AST lookahead, const v_rules& rules);
};

std::ostream& operator<<(std::ostream& stream, Tokenizer::token tok);

class Parser{
    public:
    using AST = MiniParser::AST;
    using v_AST = MiniParser::v_AST;
    using v_tokens = MiniParser::v_tokens;
    using v_rules = MiniParser::v_rules;
    static v_rules rules; /// defined in rules.cpp

    static v_AST preprocess(v_tokens); /// splits stuff into commands so parsing is easier

    static void preproc_unquote_strings(v_AST &stack);
    static void preproc_fold_curlies(v_AST &stack);
    static AST process(v_tokens); /// runs everything through MiniParser
    static void parse_braces(AST& ast);
};