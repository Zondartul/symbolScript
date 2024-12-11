#include "parser/parser.h"

Parser::AST ast(std::string name, std::vector<Parser::AST> children={});
Parser::AST ast_literal(std::string text);
bool rule_matches_template(MiniParser::rule R, Parser::AST templ);
int get_rule_index(const std::vector<MiniParser::rule>& rules, const Parser::AST& templ);
// -1 means SHIFT next token onto stack
// 0..N means REDUCE using rule N
extern const int RULE_IDX_SHIFT;

std::vector<int> template_to_rule_sequence(
    const std::vector<MiniParser::rule>& rules, const Parser::AST& templ);