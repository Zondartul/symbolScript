#pragma once
#include "parser/parser.h"
#include "util/sym.h"
#include "interpreter/commands/cmd.h"
#include <optional>

struct analysis_var_def{
    std::string name;
    std::optional<std::string> type;
    std::optional<std::string> description;
    std::optional<Sym> constructor_sym;
    std::optional<Sym> assigned_sym;
    int assignment_mode = 0;
};

class Analyzer{
    public:
        using AST = MiniParser::AST;
        Analyzer();

        bool debug = false;

        CmdList analyze(AST ast);          /// read the tree and generate commands
        Sym analyze_sym(AST ast);       /// read a tree and generate a symbol
        Sym analyze_sym_sym(AST ast);   /// read a "sym" tree and generate a symbol

        void assert_arity(AST ast, int n_min, int n_max);
        std::string get_ident(AST ast);
        float get_float(Sym s);
        CmdList get_CmdList(Sym s);
        CmdList analyze_var_definition(AST ast);
        void analyze_vd_options(analysis_var_def& vd, AST ast);
};
