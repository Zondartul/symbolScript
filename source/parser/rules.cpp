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
     
     {{"\\if", "braced_sym", "statement"},         "*",     "if_block"},    //0
     
     {{"if_block"},                           "\\elif",      "SHIFT"},      //1
     {{"if_block", "\\elif", "statement"},         "*",     "elif_block"},  //2
     
     {{"elif_block"},                         "\\elif", "SHIFT"},           //3
     {{"elif_block", "\\elif", "statement"},       "*",     "elif_block"},  //4
     
     {{"if_block"},                           "\\else",      "SHIFT"},      //5
     {{"if_block", "\\else", "statement"},         "*",     "if_else_block"},//6
     
     {{"elif_block", "\\else", "statement"},       "*",     "if_else_block"},//7
     
     {{"if_block"},                           "*",      "statement"},//8
     {{"elif_block"},                         "*",      "statement"},//9
     {{"if_else_block"},                      "*",      "statement"},//10
     

     {{"\\(", "sym", "\\)"},                "*",     "braced_sym"},//11
     {{"\\(", "sym_list", "\\)"},           "*",     "braced_sym_list"},//12
      {{"sym", "\\,", "sym"},               "OP",    "SHIFT"},//13
      {{"sym_list", "\\,", "sym"},          "OP",    "SHIFT"},//14
      {{"sym", "\\,", "sym"},               "*",     "sym_list"},//15
      {{"sym_list", "\\,", "sym"},          "*",     "sym_list"},//16
    {{"\\:=", "sym"},                       "OP",    "SHIFT"},//17
    {{"\\=", "sym"},                        "OP",    "SHIFT"},//18
    {{"\\:=", "sym"},                       "\\[",   "SHIFT"},//19
    {{"\\=", "sym"},                        "\\[",   "SHIFT"},//20
    {{"sym", "\\:", "IDENT"},                 "*",     "type_hint"},//21
    {{"type_hint"},                         "*",     "sym"},//22
    {{"sym", "\\=", "sym"},                "\\;",   "assignment"},//23
    {{"sym", "\\:=", "sym"},               "\\;",   "assignment"},//24
    //{{"sym", "\\=", "function_call"},      "\\;",   "assignment"},
    //{{"sym", "\\:=", "function_call"},     "\\;",   "assignment"},
    {{"\\:=", "sym"},                       "*",     "vd_assignment"},//25
    {{"\\=", "sym"},                        "*",     "vd_assignment"},//26
    //{{"\\:=", "function_call"},             "\\;",   "vd_assignment"},
    //{{"\\=",  "function_call"},             "\\;",   "vd_assignment"},    
    {{"\\--", "STRING"},                    "*",     "vd_description"},//27
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
    {{"assignment", "\\;"},                 "*",   "statement"},//28
    /// ---------------
    /// function_call: sym '(' sym_list ')' ';'
    {{"IDENT"},                             "\\(",   "SHIFT"},//29
    {{"IDENT"},                             "*",     "sym"},//30
    {{"\\(", "\\)"},                        "*",     "braces_empty"},//31
    {{"IDENT", "braces_empty"},             "*",     "function_call"},//32
    {{"IDENT", "braced_sym"},               "*",     "function_call"},//33
    {{"IDENT", "braced_sym_list"},          "*",     "function_call"},//34
    /// ---------------
    /// array: '[' sym_list ']'
    {{"\\[", "\\]"},                        "*",     "array"},//35
    {{"\\[","sym","\\]"},                   "*",     "array"},//36
    {{"\\[", "sym_list", "\\]"},            "*",     "array"},//37
    {{"ARRAY", "array"},                    "*",     "array_constr"}, //38 /// array constructor
    {{"array_constr"},                      "*",     "sym"},//39
    /// array_list: [array (',' array)]
    {{"array", "\\,", "array"},             "*",     "array_list"},//40
    {{"array_list", "\\,", "array"},        "*",     "array_list"},//41
        /// vector_constr: 'vector' array
        //{{"VECTOR", "array"},                   "*",     "vector_constr"},
        //{{"vector_constr"},                     "*",     "sym"},
        /// matrix_constr: 'matrix' '[' array_list ']'
        //{{"MATRIX", "\\[", "\\]"},              "*",     "matrix_constr"}, // 0-by-0
        //{{"MATRIX", "\\[", "array", "\\]"},     "*",     "matrix_constr"}, // n-by-1
        //{{"MATRIX", "\\[", "array_list", "\\]"},"*",     "matrix_constr"}, // n-by-m
        //{{"matrix_constr"},                     "*",     "sym"},
        /// command_constr: '{' command* '}'
    {{"\\{", "command_list", "\\}"},        "*",     "command_constr"},//42
        {{"statement", "statement"},        "*",     "command_list"},//43
        {{"command_list", "statement"},     "*",     "command_list"},//44
    /// sym is every expression except raw IDENT
    {{"NUMBER"},                            "*",     "sym"},//45 /// 123
    {{"STRING"},                            "*",     "sym"},//46 /// "abc"
    {{"{}"},                                "*",     "sym"},//47 /// { cmd1; cmd2; } /// need to parse things inside btw
    {{"sym", "OP", "sym"},                  "\\[",   "SHIFT"},//48 /// a+ b[...] SHIFT
    {{"sym", "OP", "sym"},                  "*",     "sym"},//49 /// a+b
    {{"OP", "sym"},                         "*",     "sym"},//50 /// -a
    {{"\\if", "braced_sym"},                "*",     "SHIFT"},//51 /// if(a)...
    {{"braced_sym"},                        "*",     "sym"},//52 /// (a+b)
    {{"function_call"},                     "*",     "sym"},//53 /// foo(a,b,c)
    {{"sym", "array"},                      "*",     "sym"},//54 /// a[b] = c[d]
    //{{"sym"},                               "\\;",   "statement"}, /// e.g. if it's a function call
    {{"sym", "\\;"},                        "*",     "statement"},//55 /// e.g. if it's a function call
    
    {{"statement_list", "statement"},       "*",     "statement_list"},//56
    {{"statement"},                         "*",     "statement_list"},//57
    //{{"statement_list", "statement_list"},  "*",     "statement_list"},
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
                ast("\\;")
            }),
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
                }),
                ast("\\;")
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
                ast("if_block",{
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
                                    ast("sym",{ast("IDENT")}),
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