#include "parser/parser.h"
#include "parser/parse_template.h"
/// SymbolScript syntax
/// keywords:
/// definition:     class func interface
/// flow control:   while for in if elif else
/// punctuation:    ( , ) { ; } [ : ]
/// comments: // /* */
/// literals: numeric: -1 -1.0e-5ijk 0b1011 0xF00B 
///            string: "blah" % x .. stuff
///
Tokenizer::v_minitoken_infos Tokenizer::tok_dict{
    //{std::regex("vector"), "VECTOR"},
    //{std::regex("matrix"), "MATRIX"},
    //{std::regex("array"),  "ARRAY"},
    {std::regex("\"[^\"]*\""), "STRING"},
    {std::regex("//.*\n|//.*$"), "COMMENT"},
    {std::regex("\\/\\*([^*]|\\*[^\\/])*(\\*\\/|$)"), "COMMENT"},
    {std::regex("and|or|not|in"), "OP"},
    {std::regex("if|elif|else|while|for|return"), "KEYWORD"},
    {std::regex("[A-Za-z][_a-zA-Z0-9]*"), "IDENT"},
    {std::regex("=="), "OP"},
    {std::regex(":=|[:=(){};\\[\\],]|--"), "PUNCT"},
    {std::regex("[+\\-\\/^%<>.*!?&]"), "OP"},
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

    /// old rules:
    /// var_defintion(expanded):
    ///   IDENT [':' IDENT ['(' sym_list ')']] [ (':='|'=') sym ] [ '--' STRING ] ';'
    ///   -name-  -type-    -constr.args-     -copy-or-func-init- -description-
    ///
    /// var_definition: IDENT type_constr? initializer? description?
    /// prefix,    lookeahead,   reduction
    
    /// new rules:
    
    //{{"\\:", "IDENT"},                      "\\(",     "SHIFT"},
    //{{"\\:", "IDENT"},                      "*",     "vd_type_hint"},
    //{{"\\:", "IDENT"},                      "\\(",   "SHIFT"},
    //{{"IDENT"},                            "\\:",    "SHIFT"},
    //{{"IDENT"},                            "\\:=",    "SHIFT"},
    //{{"IDENT"},                            "\\=",    "SHIFT"},
    //{{"\\:", "IDENT", "braced_sym"},        "*",     "vd_type_hint"},
    //{{"\\:", "IDENT", "braced_sym_list"},   "*",     "vd_type_hint"}, /// n args
     //if-elif-else block
     //
     //        /\ 
     //        v ^
     /// if -> elif -> else
     //    \      \----/
     //     \---------/
     
     {{"\\if", "braced_sym", "statement"},         "*",     "if_block"},
     
     {{"if_block"},                           "\\elif",      "SHIFT"},
     {{"if_block", "\\elif", "statement"},         "*",     "elif_block"},
     
     {{"elif_block"},                         "\\elif", "SHIFT"},
     {{"elif_block", "\\elif", "statement"},       "*",     "elif_block"},
     
     {{"if_block"},                           "\\else",      "SHIFT"},
     {{"if_block", "\\else", "statement"},         "*",     "if_else_block"},
     
     {{"elif_block", "\\else", "statement"},       "*",     "if_else_block"},
     
     {{"if_block"},                           "*",      "statement"},
     {{"elif_block"},                         "*",      "statement"},
     {{"if_else_block"},                      "*",      "statement"},
     

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
    {{"sym", "\\:", "IDENT"},                 "*",     "type_hint"},
    {{"type_hint"},                         "*",     "sym"},
    {{"sym", "\\=", "sym"},                "\\;",   "assignment"},
    {{"sym", "\\:=", "sym"},               "\\;",   "assignment"},
    //{{"sym", "\\=", "function_call"},      "\\;",   "assignment"},
    //{{"sym", "\\:=", "function_call"},     "\\;",   "assignment"},
    {{"\\:=", "sym"},                       "*",     "vd_assignment"},
    {{"\\=", "sym"},                        "*",     "vd_assignment"},
    //{{"\\:=", "function_call"},             "\\;",   "vd_assignment"},
    //{{"\\=",  "function_call"},             "\\;",   "vd_assignment"},    
    {{"\\--", "STRING"},                    "*",     "vd_description"},
        /// collecting all the opts
        //{{"vd_type_hint"},                      "\\;",   "vd_options"},
        //{{"vd_type_hint"},                      "\\--",  "vd_options3"},
        //{{"vd_type_hint"},                      "\\=",   "vd_options2"},
        //{{"vd_type_hint"},                      "\\:=",  "vd_options2"},

        //{{"vd_assignment"},                      "\\;",  "vd_options"},
        //{{"vd_assignment"},                      "\\--", "vd_options3"},
        
        //{{"vd_description"},                    "\\;",   "vd_options"},
        /// collapse all the opts
        //{{"vd_options2", "vd_options3"},        "*",     "vd_options3"},
        //{{"vd_options2", "vd_options"},         "*",     "vd_options"},
        //{{"vd_options3", "vd_options"},         "*",     "vd_options"},

        //{{"IDENT", "vd_options"},               "\\;",   "var_definition"},

        //{{"var_definition"},                    "\\;",   "statement"},
    //{{"assignment"},                        "\\;",   "statement"},
    {{"assignment", "\\;"},                 "*",   "statement"},
    /// ---------------
    /// function_call: sym '(' sym_list ')' ';'
    {{"IDENT"},                             "\\(",   "SHIFT"},
    {{"IDENT"},                             "*",     "sym"},
    {{"\\(", "\\)"},                        "*",     "braces_empty"},
    {{"IDENT", "braces_empty"},             "*",     "function_call"},
    {{"IDENT", "braced_sym"},               "*",     "function_call"},
    {{"IDENT", "braced_sym_list"},          "*",     "function_call"},
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
        //{{"VECTOR", "array"},                   "*",     "vector_constr"},
        //{{"vector_constr"},                     "*",     "sym"},
        /// matrix_constr: 'matrix' '[' array_list ']'
        //{{"MATRIX", "\\[", "\\]"},              "*",     "matrix_constr"}, // 0-by-0
        //{{"MATRIX", "\\[", "array", "\\]"},     "*",     "matrix_constr"}, // n-by-1
        //{{"MATRIX", "\\[", "array_list", "\\]"},"*",     "matrix_constr"}, // n-by-m
        //{{"matrix_constr"},                     "*",     "sym"},
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
    //{{"sym"},                               "\\;",   "statement"}, /// e.g. if it's a function call
    {{"sym", "\\;"},                        "*",     "statement"}, /// e.g. if it's a function call
    
    {{"statement"},                         "*",     "statement_list"},
    //{{"statement_list", "statement_list"},  "*",     "statement_list"},
    {{"statement_list", "statement"},       "*",     "statement_list"},
    ///-------------------
    
};

std::vector<std::pair<std::string, Parser::AST>> Parser::tests = {
    {"A = B;",
        ast("statement_list",{
            ast("statement", {
                ast("assignment", {
                    ast("sym",{ast("IDENT")}),
                    ast("\\="),
                    ast("sym",{ast("IDENT")})
                }), 
            }),
            ast("\\;")
        })
    },
    {"B := 5+4*3+(2*1.5);", 
        ast("statement_list",{
            ast("statement",{
                ast("assignment",{
                    ast("sym",{ast("IDENT")}),
                    ast("\\:="),
                    ast("sym",{
                        ast("sym",{
                            ast("sym",{
                                ast("sym",{ast("NUMBER")}),
                                ast("OP"),
                                ast("sym",{ast("NUMBER")})
                            }),
                            ast("OP"),
                            ast("sym",{ast("NUMBER")})
                        }),
                        ast("OP"),
                        ast("sym",{
                            ast("braced_sym",{
                                ast("\\("),
                                ast("sym",{
                                    ast("sym",{ast("NUMBER")}),
                                    ast("OP"),
                                    ast("sym",{ast("NUMBER")}),
                                }),
                                ast("\\)")
                            })
                        })
                    })
                })
            })
        })
    },
    {"print(\"hello\");",
        ast("statement_list",{
            ast("statement",{
                ast("sym",{
                    ast("function_call",{
                        ast("IDENT"),
                        ast("braced_sym",{
                            ast("\\("),
                            ast("sym",{ast("STRING")}),
                            ast("\\)"),
                        })
                    })
                }),
                ast("\\;")
            })
        })
    },
    {"C = 10 + sqrt(A);",
        ast("statement_list",{
            ast("statement",{
                ast("assignment",{
                    ast("sym",{ast("IDENT")}),
                    ast("\\="),
                    ast("sym",{
                        ast("sym",{ast("NUMBER")}),
                        ast("OP"),
                        ast("sym",{
                            ast("function_call",{
                                ast("IDENT"),
                                ast("braced_sym",{
                                    ast("\\("),
                                    ast("sym",{ast("IDENT")}),
                                    ast("\\)")
                                })
                            })
                        })
                    })
                }),
                ast("\\;")
            })
        })
    },
    {"if(A) print(B);", 
        ast("statement_list",{
            ast("statement",{
                ast("if-block",{
                    ast("\\if"),
                    ast("braced_sym",{
                        ast("\\("),
                        ast("sym",{ast("IDENT")}),
                        ast("\\)")
                    }),
                    ast("statement",{
                        ast("sym",{
                            ast("function_call",{
                                ast("IDENT"),
                                ast("braced_sym",{
                                    ast("\\("),
                                    ast("sym",{ast("STRING")}),
                                    ast("\\)")
                                })
                            })
                        }),
                        ast("\\;")
                    })
                })
            })
        })
    }
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