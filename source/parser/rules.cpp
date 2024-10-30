#include "parser/parser.h"

Tokenizer::v_minitoken_infos Tokenizer::tok_dict{
    {std::regex("vector"), "VECTOR"},
    {std::regex("matrix"), "MATRIX"},
    {std::regex("array"),  "ARRAY"},
    {std::regex("\"[^\"]*\""), "STRING"},
    {std::regex("//.*\n"), "COMMENT"},
    {std::regex("and|or|not"), "OP"},
    {std::regex("[A-Za-z][_a-zA-Z0-9]*"), "IDENT"},
    {std::regex("=="), "OP"},
    {std::regex(":=|[:=(){};\\[\\],]|--"), "PUNCT"},
    {std::regex("[+\\-\\/^<>.*!]"), "OP"},
    {std::regex("\\-?[0-9]+(\\.[0-9]+)?(e-?[0-9]+)?"), "NUMBER"},
    //{std::regex("[+\\-\\/*]"), "OP"},
    {std::regex("[ \t\r]+"), "SP"},
    {std::regex("\n"), "NL"}
};

Parser::v_rules Parser::rules = {
    /*
    // test rules while we figure out why the thing as a whole doesn't work
    {{"\\--", "STRING"}, "description"}
    */

    /// pst: i really should add a look-ahead symbol so I know when to shift or reduce
    ///  i.e. when to close :IDENT.; -> reduce or :IDENT.( -> shift

    /// also later, add a list of NT-tokens to flatten (temporary tokens that only force an ordering)

    // true rules here:
    /// var_defintion(expanded):
    ///   IDENT [':' IDENT ['(' sym_list ')']] [ (':='|'=') sym ] [ '--' STRING ] ';'
    ///   -name-  -type-    -constr.args-     -copy-or-func-init- -description-
    ///
    /// var_definition: IDENT type_constr? initializer? description?
    /// prefix,    lookeahead,   reduction
    //{{"sym"},                      "\\;",     "var_definition"},
    {{"\\:", "IDENT"},                      "\\(",     "SHIFT"},
    {{"\\:", "IDENT"},                      "*",     "vd_type_hint"},
    {{"\\:", "IDENT"},                      "\\(",   "SHIFT"},
    {{"IDENT"},                            "\\:",    "SHIFT"},
    {{"IDENT"},                            "\\:=",    "SHIFT"},
    {{"IDENT"},                            "\\=",    "SHIFT"},
    //{{"vd_type_hint", "braced_sym"}, "vd_type_hint_w_args"},
    //{{"vd_type_hint", "braced_sym_list"},   "vd_type_hint_w_args"}, /// n args
    {{"\\:", "IDENT", "braced_sym"},        "*",     "vd_type_hint"},
    {{"\\:", "IDENT", "braced_sym_list"},   "*",     "vd_type_hint"}, /// n args
     {{"\\(", "sym", "\\)"},                "*",     "braced_sym"},
     {{"\\(", "sym_list", "\\)"},           "*",     "braced_sym_list"},
      {{"sym", "\\,", "sym"},               "OP",    "SHIFT"},
      {{"sym_list", "\\,", "sym"},          "OP",    "SHIFT"},
      {{"sym", "\\,", "sym"},               "*",     "sym_list"},
      {{"sym_list", "\\,", "sym"},          "*",     "sym_list"},
    {{"\\:=", "sym"},                       "OP",    "SHIFT"},
    {{"\\=", "sym"},                        "OP",    "SHIFT"},
    {{"\\:=", "sym"},                       "\\[",   "SHIFT"},
    {{"\\=", "sym"},                        "\\[",   "SHIFT"},
    {{"sym", "\\=", "sym"},                "\\;",   "assignment"},
    {{"sym", "\\:=", "sym"},               "\\;",   "assignment"},
    {{"sym", "\\=", "function_call"},      "\\;",   "assignment"},
    {{"sym", "\\:=", "function_call"},     "\\;",   "assignment"},
    {{"\\:=", "sym"},                       "*",     "vd_assignment"},
    {{"\\=", "sym"},                        "*",     "vd_assignment"},
    {{"\\:=", "function_call"},             "\\;",   "vd_assignment"},
    {{"\\=",  "function_call"},             "\\;",   "vd_assignment"},    
    {{"\\--", "STRING"},                    "*",     "vd_description"},
    /// collecting all the opts
    {{"vd_type_hint"},                      "\\;",   "vd_options"},
    {{"vd_type_hint"},                      "\\--",  "vd_options3"},
    {{"vd_type_hint"},                      "\\=",   "vd_options2"},
    {{"vd_type_hint"},                      "\\:=",  "vd_options2"},

    {{"vd_assignment"},                      "\\;",  "vd_options"},
    {{"vd_assignment"},                      "\\--", "vd_options3"},
    
    {{"vd_description"},                    "\\;",   "vd_options"},
    /// collapse all the opts
    {{"vd_options2", "vd_options3"},        "*",     "vd_options3"},
    {{"vd_options2", "vd_options"},         "*",     "vd_options"},
    {{"vd_options3", "vd_options"},         "*",     "vd_options"},

    {{"IDENT", "vd_options"},               "\\;",   "var_definition"},

    {{"var_definition"},                    "\\;",   "statement"},
    {{"assignment"},                        "\\;",   "statement"},
    //{{"sym", "vd_assignment"},              "\\;",   "assignment"},
    //{{"assignment"},                        "\\;",   "statement"},
    /// ---------------
    /// function_call: sym '(' sym_list ')' ';'
    {{"IDENT"},                             "\\(",   "SHIFT"},
    {{"IDENT"},                             "*",     "sym"},
    {{"\\(", "\\)"},                        "*",     "braces_empty"},
    //{{"IDENT", "\\(", "\\)"},               "*",     "function_call"},
    {{"IDENT", "braces_empty"},             "*",     "function_call"},
    {{"IDENT", "braced_sym"},               "*",     "function_call"},
    {{"IDENT", "braced_sym_list"},          "*",     "function_call"},
    //{{"function_call"},                     "\\;",   "statement"},
    //{{"sym"},                               "\\;",  "statement"}, /// eh, might be a function | actually foo; shouldn't be a thing.
    /// ---------------
    /// array: '[' sym_list ']'
    {{"\\[", "\\]"},                        "*",     "array"},
    {{"\\[","sym","\\]"},                   "*",     "array"},
    {{"\\[", "sym_list", "\\]"},            "*",     "array"},
    {{"ARRAY", "array"},                    "*",     "array_constr"}, /// array constructor
    {{"array_constr"},                      "*",     "sym"},
    /// array_list: [array (',' array)]
    {{"array", "\\,", "array"},             "*",     "array_list"},
    {{"array_list", "\\,", "array"},        "*",     "array_list"},
    /// vector_constr: 'vector' array
    {{"VECTOR", "array"},                   "*",     "vector_constr"},
    {{"vector_constr"},                     "*",     "sym"},
    /// matrix_constr: 'matrix' '[' array_list ']'
    {{"MATRIX", "\\[", "\\]"},              "*",     "matrix_constr"}, // 0-by-0
    {{"MATRIX", "\\[", "array", "\\]"},     "*",     "matrix_constr"}, // n-by-1
    {{"MATRIX", "\\[", "array_list", "\\]"},"*",     "matrix_constr"}, // n-by-m
    {{"matrix_constr"},                     "*",     "sym"},
    /// command_constr: '{' command* '}'
    {{"\\{", "command_list", "\\}"},        "*",     "command_constr"},
        {{"statement", "statement"},        "*",     "command_list"},
        {{"command_list", "statement"},     "*",     "command_list"},
    /// sym is every expression except raw IDENT
    {{"NUMBER"},                            "*",     "sym"}, /// 123
    {{"STRING"},                            "*",     "sym"}, /// "abc"
    {{"{}"},                                "*",     "sym"}, /// { cmd1; cmd2; } /// need to parse things inside btw
    {{"sym", "OP", "sym"},                  "\\[",   "SHIFT"}, /// a+ b[...] SHIFT
    {{"sym", "OP", "sym"},                  "*",     "sym"}, /// a+b
    {{"OP", "sym"},                         "*",     "sym"}, /// -a
    {{"braced_sym"},                        "*",     "sym"}, /// (a+b)
    {{"function_call"},                     "*",     "sym"}, /// foo(a,b,c)
    {{"sym", "array"},                      "*",     "sym"}, /// a[b] = c[d]
    {{"sym"},                               "\\;",   "statement"}, /// e.g. if it's a function call
    {{"statement", "\\;"},                  "*",     "statement_list"},
    {{"statement_list", "statement_list"},  "*",     "statement_list"},
    ///-------------------
    
};

  /*
    {{"IDENT", "var_def_rest"}, "var_definition"},
        /// :type
        {{"\\:", "IDENT"},      "var_def_rest"},
        /// (constructor args)
        {{"var_def_rest", "braced_sym"},   "var_def_rest"}, /// 1 arg
            {{"\\(", "sym", "\\)"}, "braced_sym"},
        {{"var_def_rest", "braced_sym_list"},   "var_def_rest"}, /// n args
            {{"\\(", "sym_list", "\\)"}, "braced_sym_list"},
                {{"sym", "\\,", "sym"}, "sym_list"},
                {{"sym_list", "\\,", "sym"}, "sym_list"},
        /// initialization: (:= | =) sym
        {{"var_def_rest", "assign_copy"},   "var_def_rest"},
            {{"\\:=", "sym_expr"}, "assign_copy"},
        {{"var_def_rest", "assign_func"},   "var_def_rest"},
            {{"\\=", "sym_expr"}, "assign_func"},
        /// -- "description"
        {{"var_def_rest", "description"},   "var_def_rest"},
            {{"\\--", "STRING"}, "description"},
    {{"IDENT", "initializer"}, "var_definition"},
    {{"var_definition", "\\;"}, "statement"},
    */