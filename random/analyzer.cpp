#include "parser/analyzer.h"

using AST = MiniParser::AST;
using v_AST = MiniParser::v_AST;
using token = Tokenizer::token;

#define DEBUG(x) if(debug){x;}

Sym flatten_sym(Sym sym);

Analyzer::Analyzer(){}

std::string toString(AST ast, int depth=-1, bool multiline=false){ /// depth: -1 means recursive, 0 means only token, 1 means immediate children, etc
    std::stringstream ss;
    ss << ast.tok;
    if(depth != 0){
        ss << ": ";
        int next_depth = (depth == -1) ? depth : depth - 1;
        if(ast.children.size()){
            if(multiline){ss << "\n";}
            for(const auto& ast2: ast.children){
                ss << toString(ast2, next_depth) << " ";
            }
            if(multiline){ss << "\n";}
        }
    } 
    return ss.str();
}

void Analyzer::assert_arity(AST ast, int n_min, int n_max){
    int n = ast.children.size();
    if((n < n_min) || (n > n_max)){
        std::stringstream ss;
        ss << "semantic error: " << ast.tok.type;
        ss << " needs to have between " << n_min;
        ss << " and " << n_max << " arguments,";
        ss << " but has " << n << "\n";
        ss << " AST: " << toString(ast, 1);
        std::cout << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
};

std::string Analyzer::get_ident(AST ast){
    assert(ast.tok.type == "IDENT");
    return ast.tok.text;
};

float Analyzer::get_float(Sym s){
    assert(s.is_const);
    return s.val;
}

CmdList Analyzer::get_CmdList(Sym s){
    if(s.name == "braced_sym"){
        Sym s2 = s.children.at(0);
        if(s2.name == "{}"){
            CmdList cl = s2.cmds;
            if(!cl.empty()){
                return cl;
            }
        }
    }else if(s.name == "{}"){
        CmdList cl = s.cmds;
        if(!cl.empty()){
            return cl;
        }
    }
    /// error
    std::stringstream ss;
    ss << "Can't read CmdList from sym: [" << s << "]" << std::endl;
    std::cout << ss.str() << std::endl;
    throw std::runtime_error(ss.str());
}

#ifndef UNIMPLMEMENTED
    #define UNIMPLEMENTED() assert(!"unimplmented")
#endif

#ifndef ERROR
    #define ERROR(x) {std::stringstream ss; ss << x; std::cout << ss.str() << std::endl; throw std::runtime_error(ss.str());}
#endif


#include "interpreter/commands/cmdValX.h"
extern std::map<std::string, const zAnyHandler*> zah_map;

bool is_valid_type(std::string S){
    return zah_map.count(S);
}


/*
int recurse_smallest_line_pos(const AST &ast, int smallest){
    if(ast.tok.pos1.line != -1){
        if(smallest == -1){
            smallest = ast.tok.pos1.line;
        }
        if(ast.tok.pos1.line < smallest){
            smallest = ast.tok.pos1.line;
        }
    }
    for(const auto& ch:ast.children){
        smallest = recurse_smallest_line_pos(ch, smallest);
    }
    return smallest;
}
*/

using tok_pos = Tokenizer::tok_pos;

tok_pos recurse_smallest_pos(const AST &ast, tok_pos cur_pos){
    if(ast.tok.pos1.char_idx != -1){
        if(cur_pos.char_idx == -1){
            cur_pos = ast.tok.pos1;
        }
        if(ast.tok.pos1.char_idx < cur_pos.char_idx){
            cur_pos = ast.tok.pos1;
        }
    }
    for(const auto& ch:ast.children){
        cur_pos = recurse_smallest_pos(ch, cur_pos);
    }
    return cur_pos;
}

tok_pos recurse_biggest_pos(const AST &ast, tok_pos cur_pos){
    if(ast.tok.pos2.char_idx != -1){
        if(cur_pos.char_idx == -1){
            cur_pos = ast.tok.pos2;
        }
        if(ast.tok.pos2.char_idx > cur_pos.char_idx){
            cur_pos = ast.tok.pos2;
        }
    }
    for(const auto& ch:ast.children){
        cur_pos = recurse_biggest_pos(ch, cur_pos);
    }
    return cur_pos;
}


void calculate_cmd_address(const AST &ast, Cmd &cmd){
    //int line_num = recurse_smallest_line_pos(ast, ast.tok.pos1.line);
    tok_pos min_pos = recurse_smallest_pos(ast, ast.tok.pos1);
    tok_pos max_pos = recurse_biggest_pos(ast, ast.tok.pos2);
    int line_num = min_pos.line;
    cmd.pos.line = line_num;
    cmd.pos.from = min_pos;
    cmd.pos.to = max_pos;
}

#define WARN_DROP(x,y) {std::cout << "warn: " << x << " silently dropped from " << y << std::endl;}

void Analyzer::analyze_vd_options(analysis_var_def& vd, AST ast){
    if(ast.tok.type == "vd_options"){
        /// {"vd_type_hint"}
        /// {"vd_assignment"}
        /// {"vd_description"}
        /// {"vd_options2", "vd_options"}
        /// {"vd_options3", "vd_options"}
        assert_arity(ast, 1,2);
        token t1 = ast.children.at(0).tok;
        if(t1.type == "vd_type_hint"){
            assert_arity(ast, 1,1);
            analyze_vd_options(vd, ast.children.at(0));
        }else if(t1.type == "vd_assignment"){
            assert_arity(ast, 1,1);
            analyze_vd_options(vd, ast.children.at(0));
        }else if(t1.type == "vd_description"){
            assert_arity(ast, 1,1);
            analyze_vd_options(vd, ast.children.at(0));
        }else if(t1.type == "vd_options2"){
            assert_arity(ast, 2,2);
            analyze_vd_options(vd, ast.children.at(0));
            analyze_vd_options(vd, ast.children.at(1));
        }else if(t1.type == "vd_options3"){
            assert_arity(ast, 2,2);
            analyze_vd_options(vd, ast.children.at(0));
            analyze_vd_options(vd, ast.children.at(1));
        }else{
            ERROR("unknown production (1)");
        }
    }else if(ast.tok.type == "vd_options3"){
        /// {"vd_type_hint"}
        /// {"vd_assignment"}
        /// {"vd_options2", "vd_options3"}
        assert_arity(ast,1,2);
        token t1 = ast.children.at(0).tok;
        if(t1.type == "vd_type_hint"){
            assert_arity(ast, 1,1);
            analyze_vd_options(vd, ast.children.at(0));
        }else if(t1.type == "vd_assignment"){
            assert_arity(ast, 1,1);
            analyze_vd_options(vd, ast.children.at(0));
        }else if(t1.type == "vd_options2"){
            assert_arity(ast, 2,2);
            analyze_vd_options(vd, ast.children.at(0));
            analyze_vd_options(vd, ast.children.at(1));
        }else{
            ERROR("unknown production (2)");
        }
    }else if(ast.tok.type == "vd_options2"){
        /// {"vd_type_hint"}
        assert_arity(ast,1,1);
        analyze_vd_options(vd, ast.children.at(0));
    }else if(ast.tok.type == "vd_type_hint"){
        // 0   1        3
        // : IDENT [braced_sym|braced_sym_list|braces_empty]
        
        /// {"\\:", "IDENT"
        /// {"\\:", "IDENT", "braced_sym"
        /// {"\\:", "IDENT", "braced_sym_list"
        assert_arity(ast, 2,3);
        std::string type = get_ident(ast.children.at(1));
        if(ast.children.size() >= 3){
            //std::cout << "type hint with constructor" << std::endl;
            const auto& ast_arg = ast.children.at(2);
            vd.constructor_sym = flatten_sym(analyze_sym(ast_arg));
        }else{
        //    std::cout << "type hint, NO constructor" << std::endl;
        }
        
        if(!is_valid_type(type)){
            std::stringstream ss;
            ss << "Not a valid type: " << type;
            std::cout << ss.str() << std::endl;
            throw std::runtime_error(ss.str());
        }
        vd.type = type;
        //std::cout << "got vd_type_hint" << std::endl;
    }else if(ast.tok.type == "vd_assignment"){
        /// (=|:=) sym
        assert_arity(ast, 2,2);
        token t1 = ast.children.at(0).tok;
        if(t1.text == "="){vd.assignment_mode = 1;}
        else if(t1.text == ":="){vd.assignment_mode = 2;}

        vd.assigned_sym = flatten_sym(analyze_sym(ast.children.at(1)));
        
        //std::cout << "got vd_assignment" << std::endl;
    }else if(ast.tok.type == "vd_description"){
        /// {"\\--", "STRING"}
        assert_arity(ast, 2,2);
        token t2 = ast.children.at(1).tok;
        assert(t2.type == "STRING");
        vd.description = t2.text;
        //std::cout << "got vd_description" << std::endl;
    }
    else{
        ERROR("unkown production (3)");
    }
}

/// to fix a bug where you can't do x := x + 1
#define INVERT_ASSIGN_ORDER

CmdList Analyzer::analyze_var_definition(AST ast){
    assert_arity(ast, 1,2);
    analysis_var_def vd;
    vd.name = get_ident(ast.children.at(0));
    
    const auto& options = ast.children.at(1);
    analyze_vd_options(vd, options);

    if(!vd.type){
        assert(vd.assigned_sym);
        assert(!vd.constructor_sym);
        vd.type = "function";
    }
    /// todo: prolly get rid of these different type commands?
    /// and just make it a single "create var" command?
    CmdList res;
    Cmd cmd_vd;

    auto do_assignment = [&](){
        if(vd.assigned_sym){
            Sym ass = *vd.assigned_sym;
            if(vd.assignment_mode == 1){
                // = means symbolic
                auto cmd_ass = Cmd::i_set_sym_s(Sym(vd.name), ass);
                calculate_cmd_address(ast, cmd_ass);
                res.push_back(cmd_ass);
            }else if(vd.assignment_mode == 2){
                // := means by value
                #ifdef INVERT_ASSIGN_ORDER
                    auto cmd_ass = Cmd::i_set_copy(Sym("tmp"), ass);
                #elif
                    auto cmd_ass = Cmd::i_set_copy(Sym(vd.name), ass);
                #endif
                calculate_cmd_address(ast, cmd_ass);
                res.push_back(cmd_ass);
            }else{
                assert(false);
            }
        }
    };

    #ifdef INVERT_ASSIGN_ORDER
        if(vd.assignment_mode == 2){do_assignment();}
    #endif
    /// d_integer
    /// d_floating
    /// d_material
    /// d_gmsh
    /// d_body
    /// d_constant
    /// d_unknown
    /// d_builtin
    /// d_function
    /// d_systemRealTrans
    /// d_sys_vector 
    /// d_sys_matrix
    /// d_dof_vector
    /// d_dof_matrix 
    /// d_array
    if(vd.type == "integer"){
        assert(!vd.description);
        int n = 0;
        if(vd.constructor_sym){
            n = get_float(*vd.constructor_sym);
        }
        cmd_vd = Cmd::d_integer(vd.name, n);
    } 
    else if(vd.type == "float"){
        assert(!vd.description);
        float f = 0;
        if(vd.constructor_sym){
            f = get_float(*vd.constructor_sym);
        }
        cmd_vd = Cmd::d_floating(vd.name, f);
    }
    else if(vd.type == "material"){
        assert(!vd.description);
        CmdList cmds;
        if(vd.constructor_sym){
            DEBUG(std::cout << "material: has cmdlist" << std::endl);
            cmds = get_CmdList(*vd.constructor_sym);
            assert(!cmds.empty());
        }else{
            DEBUG(std::cout << "material: no cmdlist" << std::endl);
        }
        cmd_vd = Cmd::d_material(vd.name, cmds);
    }
    else if(vd.type == "assyShape"){
        assert(!vd.description);
        SymList sl;
        assert(vd.constructor_sym);
        sl = vd.constructor_sym->children;
        cmd_vd = Cmd::d_assyShape(vd.name, sl);
    }
    else if(vd.type == "gmsh"){
        assert(!vd.description);
        std::string filename;
        if(vd.constructor_sym){
            filename = vd.constructor_sym->name;
            assert(!filename.empty());
        }
        cmd_vd = Cmd::d_gmsh(vd.name, filename);
    }
    else if(vd.type == "body"){
        assert(!vd.description);
        CmdList cmds;
        if(vd.constructor_sym){
            cmds = vd.constructor_sym->cmds;
            assert(!cmds.empty());
        }
        cmd_vd = Cmd::d_body(vd.name, cmds);
    }
    else if(vd.type == "const"){
        float f = 0;
        if(vd.constructor_sym){
            f = get_float(*vd.constructor_sym);
        }
        std::string desc;
        if(vd.description){
            desc = *vd.description;
            assert(!desc.empty());
        }
        cmd_vd = Cmd::d_constant(vd.name, f, desc);
    }
    else if(vd.type == "unknown"){    
        float f = 0;
        if(vd.constructor_sym){
            f = get_float(*vd.constructor_sym);
        }
        std::string desc;
        if(vd.description){
            desc = *vd.description;
            assert(!desc.empty());
        }
        cmd_vd = Cmd::d_unknown(vd.name, f, desc);
    }
    else if(vd.type == "builtin"){
        assert(!vd.constructor_sym);
        assert(!vd.description);
        cmd_vd = Cmd::d_builtin(vd.name);
    }
    else if(vd.type == "function"){
        //assert(!vd.description);
        if(vd.description){WARN_DROP("description", vd.name);}
        Sym s;
        if(vd.constructor_sym){
            s = *vd.constructor_sym;
        }else if(vd.assigned_sym){
            s = *vd.assigned_sym;
        }
        cmd_vd = Cmd::d_function(vd.name, s);
    }
    else if(vd.type == "SystemRealTrans"){
        //assert(!vd.constructor_sym);
        CmdList cmds;
        if(vd.constructor_sym){
            cmds = vd.constructor_sym->cmds;
            assert(!cmds.empty());
        }
        assert(!vd.description);
        cmd_vd = Cmd::d_systemRealTrans(vd.name, cmds);
    }
    else if(vd.type == "Vector"){//"sys_vector"){
        assert(!vd.constructor_sym);
        assert(!vd.description);
        cmd_vd = Cmd::d_sys_vector(vd.name);
    }
    else if(vd.type == "Matrix"){//"sys_matrix"){
        assert(!vd.constructor_sym);
        assert(!vd.description);
        cmd_vd = Cmd::d_sys_matrix(vd.name);
    }
    else if(vd.type == "dofVector"){//"dof_vector"){
        assert(!vd.constructor_sym);
        assert(!vd.description);
        cmd_vd = Cmd::d_dof_vector(vd.name);
    }
    else if(vd.type == "dofMatrix"){//"dof_matrix"){
        assert(!vd.constructor_sym);
        assert(!vd.description);
        cmd_vd = Cmd::d_dof_matrix(vd.name);
    }
    else if(vd.type == "array"){//"d_array"){
        SymList sl;
        assert(vd.constructor_sym);
        sl = vd.constructor_sym->children;
        cmd_vd = Cmd::d_array(vd.name, sl);
    }
    else if(vd.type == "commands"){
        CmdList cmds;
        if(vd.constructor_sym){
            DEBUG(std::cout << "commands: has cmdlist" << std::endl);
            cmds = get_CmdList(*vd.constructor_sym);
            assert(!cmds.empty());
        }else{
            DEBUG(std::cout << "commands: no cmdlist" << std::endl);
        }
        cmd_vd = Cmd::d_commands(vd.name, cmds);
    }
    else{
        /// default? idk, make it a function?
        ///cmd_vd = Cmd::d_function(vd.name, vd.constructor_sym);
        ERROR("unknown production (4)");
    }
    //std::cout << "got var definition [" << vd.name << "] of type [" << vd.type << "]" << std::endl;

    calculate_cmd_address(ast, cmd_vd);
    res.push_back(cmd_vd);

    #ifdef INVERT_ASSIGN_ORDER
        if(vd.assigned_sym){
            if(vd.assignment_mode == 2){ /// x := y --- do as tmp := y before, then x := tmp after.
                auto cmd_ass2 = Cmd::i_set_copy(Sym(vd.name), Sym("tmp"));
                calculate_cmd_address(ast, cmd_ass2);
                res.push_back(cmd_ass2);
            }else if(vd.assignment_mode == 1){
                do_assignment(); /// x = foo --- do x:= foo afterwards.
            }
        }
    #elif
        do_assignment();
    #endif

    return res;
}

#define APPEND(x,y) x.insert(x.end(), y.begin(), y.end())



CmdList Analyzer::analyze(AST ast){
    //std::cout << "analyze: " << toString(ast, 1) << std::endl;
    CmdList res;

    if(ast.tok.type == "statement_list"){
        for(const auto& ast2: ast.children){
            auto list2 = analyze(ast2);
            APPEND(res, list2);
        }
    }
    else if(ast.tok.type == "statement"){
        assert_arity(ast,1,1);
        auto list2 = analyze(ast.children.at(0));
        APPEND(res, list2);
    }else if(ast.tok.type == "sym"){
        assert_arity(ast,1,1);
        if(ast.children.at(0).tok.type != "function_call"){
            ERROR("only function calls can occur as lone expressions");
        }
        auto list2 = analyze(ast.children.at(0));
        APPEND(res, list2);
    }else if(ast.tok.type == "var_definition"){ /// IDENT [options] -> var_definition
        auto list2 = analyze_var_definition(ast);
        APPEND(res, list2);
    }else if(ast.tok.type == "function_call"){ /// IDENT braced_sym|braced_sym_list -> function_call
        assert_arity(ast,1,2);
        std::string fc_name = get_ident(ast.children.at(0));
        Sym arg;
        bool has_arg = false;
        if(ast.children.size() == 2){
            /// figure out which it is:
            AST braces = ast.children.at(1);
            if(braces.tok.type == "braces_empty"){          /// 1. {"IDENT", "braces_empty"}
                has_arg = false;
            }else if(braces.tok.type == "braced_sym"){      /// 2. {"IDENT", "braced_sym"}
                arg = flatten_sym(analyze_sym(braces));
                has_arg = true;
            }else if(braces.tok.type == "braced_sym_list"){ /// 3. {"IDENT", "braced_sym_list"} 
                arg = flatten_sym(analyze_sym(braces));
                has_arg = true;
            }
        }
        //std::cout << "got function_call "<<fc_name << " ( " << arg << " ) " << std::endl;
        Sym sym_call;
        sym_call.op = "f()";
        if(has_arg){
            sym_call.children = {Sym(fc_name), arg};
        }else{
            sym_call.children = {Sym(fc_name)};
        }
        sym_call = flatten_sym(sym_call);
        Cmd cmd_call = Cmd::i_run(sym_call);
        calculate_cmd_address(ast, cmd_call);
        res.push_back(cmd_call);

    }else if(ast.tok.type == "PUNCT"){
        if(ast.tok.text == ";"){
            //std::cout << "skip [;]" << std::endl;
        }else{
            goto label_idk;
        }
    }
    else if(ast.tok.type == "assignment"){
        ///{"sym", "\\=", "sym"}
        ///{"sym", "\\:=", "sym"}
        ///{"sym", "\\=", "function_call"}
        ///{"sym", "\\:=", "function_call"}
        assert_arity(ast, 3, 3);
        token t2 = ast.children.at(1).tok;
        int assign_mode = 0;
        if(t2.text == "="){assign_mode = 1;}
        else if(t2.text == ":="){assign_mode = 2;}
        else{ERROR("unknown production(5)");}
        Sym s1 = flatten_sym(analyze_sym(ast.children.at(0)));
        Sym s2 = flatten_sym(analyze_sym(ast.children.at(2)));
        //Sym asn;
        //asn.op = t2.text;
        //asn.children = {s1, s2};
        
        //std::cout << "got assignment: " << asn << std::endl;
        if(assign_mode == 1){ /// = means symbolic
            Cmd cmd_ass = Cmd::i_set_sym_s(s1, s2);
            calculate_cmd_address(ast, cmd_ass);
            res.push_back(cmd_ass);
        }else if(assign_mode == 2){ // := means by value
            Cmd cmd_ass = Cmd::i_set_copy(s1, s2);
            calculate_cmd_address(ast, cmd_ass);
            res.push_back(cmd_ass);
        }else{assert(false);}
    }
    else{
        label_idk:
        std::stringstream ss;
        ss << "dunno how to analyze NT token [" << ast.tok.text << "]:"<<ast.tok.type;
        std::cout << "Semantic error: " << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }

    return res;
}

std::map<char, char> escape_chars{
    {'n','\n'},
    {'b','\b'},
    {'t','\t'},
    {'r','\r'},
    {'\\','\\'},
};

std::string unescape(std::string S){
    std::string S2;
    for(unsigned int i = 0; i < S.size(); i++){
        char c = S[i];
        if(c == '\\'){
            i++;
            if(i >= S.size()){ERROR("unfinished escape sequence in STRING");}
            c = S[i];
            if(!escape_chars.count(c)){ERROR("unknown escape sequence in STRING");}
            S2 += escape_chars[c];
        }else{
            S2 += c;
        }
    }
    std::cout << "string ["<<S<<"] unescaped as ["<<S2<<"]" << std::endl;
    return S2;
}

Sym Analyzer::analyze_sym_sym(AST ast){
    //here: {"NUMBER"} /// 123
    //here: {"STRING"} /// "abc"
    //? {"{}"}/// { cmd1; cmd2; } /// need to parse things inside btw
    //here: {"sym", "OP", "sym"}, /// a+b
    //here: {"OP", "sym"},        /// -a
    //in general: {"braced_sym"},       /// (a+b)
    //in general: {"function_call"},    /// foo(a,b,c)
    //here: {"sym", "array"},     /// a[b] = c[d]
    //in general: {"matrix_constr"},    
    //in general: {"vector_constr"},
    assert_arity(ast, 1, 3);    
    Sym res;
    token t1 = ast.children.at(0).tok;
    if(t1.type == "IDENT"){
        res.name = t1.text;
        //std::cout << "got (bar)" << std::endl;
    }else if(t1.type == "NUMBER"){
        res.is_const = true;
        std::stringstream ss(t1.text);
        ss >> res.val;
        //std::cout << "got (123)" << std::endl;
    }else if(t1.type == "STRING"){
        res.is_const = true;
        res.name = unescape(t1.text);
        //std::cout << "got (\"...\")" << std::endl;
    }else if(t1.type == "{}"){ /// code block
        /// code block should have a single "statement_list"
        /// after post-processing.
        assert_arity(ast, 1,1);
        AST ast1 = ast.children.at(0).children.at(0);
        assert(ast1.tok.type == "statement_list");
        /// at this point we would grab the output of Analyze
        /// (commands i guess)
        /// and put them into vd info
        /// as command-list data
        //#error LOOK HERE LOL
        CmdList cl = analyze(ast1);
        assert(!cl.empty());
        res.name = "{}";
        res.cmds = cl;
        //std::cout << "got (code block)" << std::endl;
    }else if(t1.type == "sym"){
        assert(ast.children.size() > 1);
        token t2 = ast.children.at(1).tok;
        if(t2.type == "OP"){
            assert_arity(ast, 3,3);
            Sym s1 = analyze_sym(ast.children.at(0));
            Sym s2 = analyze_sym(ast.children.at(2));
            res.children = {s1, s2};
            res.op = t2.text;
            //std::cout << "got (sym OP sym)" << std::endl;
        }else if(t2.type == "array"){
            assert_arity(ast, 2,2);
            Sym s1 = analyze_sym(ast.children.at(0));
            Sym s2 = analyze_sym(ast.children.at(1));
            res.children = {s1, s2};
            res.op = "[]";
            res.debug_hint = "a552";
            //std::cout << "got (sym[sym])" << std::endl;
        }else{
            ERROR("unknown production (6)");
        }
    }else if(t1.type == "OP"){
        assert_arity(ast, 2,2);
        Sym s1 = analyze_sym(ast.children.at(1));
        res.children = {s1};
        res.op = t1.text;
        //std::cout << "got (op sym)" << std::endl;
    }else if(t1.type == "braced_sym"){
        assert_arity(ast, 1,1);
        res = analyze_sym(ast.children.at(0));
    }else if(t1.type == "function_call"){
        assert_arity(ast, 1,1);
        res = analyze_sym(ast.children.at(0));
    }else if(t1.type == "matrix_constr"){
        assert_arity(ast, 1,1);
        res = analyze_sym(ast.children.at(0));
    }else if(t1.type == "vector_constr"){
        assert_arity(ast, 1,1);
        res = analyze_sym(ast.children.at(0));
    }else if(t1.type == "array_constr"){
        assert_arity(ast, 1,1);
        res = analyze_sym(ast.children.at(0));
    }else{
        ERROR("unknown production (7)");
    }
    return res;
}


/// ------------- AST flattening / list unrolling thing for symbols ---------------
/// todo: try to make it recursively print to check if it actually tries downstream symbols
SymList flatten_sym_helper(Sym sym);
void flatten_sym_helper_yes(SymList &out, Sym in);
void flatten_sym_helper_no(SymList &out, Sym in);
SymList flatten_sym_helper(Sym sym);
Sym flatten_sym(Sym sym);

unsigned int dbg_flatten_indent = 0;
#define INDENT(x) std::string(x, ' ')
/// flatten the symbol, making any lists not recursive but iterative 
Sym flatten_sym(Sym sym){
    //std::cout << INDENT(dbg_flatten_indent);
    //std::cout << "flatten before: " << sym << std::endl;

    SymList sl; flatten_sym_helper_no(sl, sym);
    Sym res = sl[0];

    //std::cout << INDENT(dbg_flatten_indent);
    //std::cout << "flatten after: " << res << std::endl;
    //std::cout << std::endl;

    return res;
}

SymList flatten_sym_helper(Sym sym, Sym parent){
    SymList res;
    //std::set<std::string> flatten_these{",", "[]"};
    //if(flatten_these.count(sym.op) && (sym.op == parent.op)){
    if(((sym.op == "[]") && (parent.op == "[]"))
        || (sym.op == ",")){
        flatten_sym_helper_yes(res, sym);
    }else{
        flatten_sym_helper_no(res, sym);
    }
    return res;
}

/// yes, rip out it's contents
void flatten_sym_helper_yes(SymList &out, Sym in){
    //std::cout << INDENT(dbg_flatten_indent++);
    //std::cout << "yes flatten on " << in << std::endl;
    for(auto s:in.children){
        SymList l2 = flatten_sym_helper(s, in);
        out.insert(out.end(), l2.begin(), l2.end());
    }
    dbg_flatten_indent--;
}

/// no, keep it as is but flatten each child
void flatten_sym_helper_no(SymList &out, Sym in){
    //std::cout << INDENT(dbg_flatten_indent++);
    //std::cout << "no flatten on " << in << std::endl;
    Sym s = in;
    s.children.clear();
    for(auto s2:in.children){
        SymList sl = flatten_sym_helper(s2, in);
        s.children.insert(s.children.end(), sl.begin(), sl.end());
    }
    out.push_back(s); 
    dbg_flatten_indent--;
}



//---------------- end flattening thing --------------

Sym Analyzer::analyze_sym(AST ast){
    Sym res;
    //std::cout << "analyze_sym " << toString(ast,1) << std::endl; 
    if(ast.tok.type == "IDENT"){
        assert_arity(ast, 0, 0);
        res = Sym(ast.tok.text);
        //std::cout << "got sym (IDENT)" << std::endl;
    }
    else if(ast.tok.type == "braced_sym"){
        /// 0  1  2
        /// ( sym )
        assert_arity(ast, 3, 3);
        res = analyze_sym(ast.children.at(1));
        //std::cout << "got sym (braced_sym)" << std::endl;
    }
    else if(ast.tok.type == "function_call"){
        /// {"IDENT", "braces_empty"}
        /// {"IDENT", "braced_sym"}
        /// {"IDENT", "braced_sym_list"}
        assert_arity(ast, 2,2);
        Sym s1 = analyze_sym(ast.children.at(0));
        if(ast.children.at(1).tok.type == "braces_empty"){
            res.children = {s1};
        }else{
            Sym s2 = analyze_sym(ast.children.at(1));
            res.children = {s1, s2};
        }
        res.op = "f()";
        //std::cout << "got (f(...))" << std::endl;
    }
    else if(ast.tok.type == "sym"){
        res = analyze_sym_sym(ast);
    }
    else if(ast.tok.type == "matrix_constr"){
        //{"MATRIX", "\\[", "\\]"}               // 0-by-0
        //{"MATRIX", "\\[", "array", "\\]"}      // n-by-1
        //{"MATRIX", "\\[", "array_list", "\\]"} // n-by-m
        assert_arity(ast, 3, 4);
        token t3 = ast.children.at(2).tok;
        if(t3.type == "PUNCT"){
            /// no args
        }else if(t3.type == "array"){
            Sym s1 = analyze_sym(ast.children.at(2));
            res.children = {s1};
            res.op = "matrix_constr";
        }else if(t3.type == "array_list"){
            Sym s1 = analyze_sym(ast.children.at(2));
            res.children = {s1};
            res.op = "matrix_constr";
        }else{
            ERROR("unknown production(8)");
        }
        //std::cout << "got (matrix[...])" << std::endl;
    }
    else if(ast.tok.type == "vector_constr"){
        //{"VECTOR", "array"} 
        assert_arity(ast, 2,2);
        Sym s1 = analyze_sym(ast.children.at(1));
        res.children = {s1};
        res.op = "vector_constr";
        //std::cout << "got (vector[...])" << std::endl;
    }
    else if(ast.tok.type == "array_constr"){
        //{"ARRAY", "array"}
        assert_arity(ast, 2,2);
        Sym s1 = analyze_sym(ast.children.at(1));
        res.children = {s1};
        res.op = "array_constr";
    }
    else if(ast.tok.type == "array"){
        /// {"\\[", "\\]"}
        /// {"\\[","sym","\\]"}
        /// {"\\[", "sym_list", "\\]"}
        assert_arity(ast, 2, 3);
        token t2 = ast.children.at(1).tok;
        res.op = "[]";////"array";
        res.debug_hint = "a656";
        if(t2.text == "]"){
            /// skip
        }else if(t2.type == "sym"){
            Sym s1 = analyze_sym(ast.children.at(1));
            res.children = {s1};
        }else if(t2.type == "sym_list"){
            Sym s1 = analyze_sym(ast.children.at(1));
            res.children = {s1};
        }else{
            ERROR("unknown production(9)");
        }
        //std::cout << "got ([...])" << std::endl;
    }
    else if(ast.tok.type == "array_list"){
        /// {"array", "\\,", "array"}
        /// {"array_list", "\\,", "array"}
        assert_arity(ast, 3,3);
        Sym s1 = analyze_sym(ast.children.at(0));
        Sym s2 = analyze_sym(ast.children.at(2));
        res.op = ",";
        res.children = {s1, s2};
        //std::cout << "got ([],[])" << std::endl;
    }
    else if(ast.tok.type == "sym_list"){
        /// {"sym", "\\,", "sym"}
        /// {"sym_list", "\\,", "sym"}
        assert_arity(ast, 3,3);
        Sym s1 = analyze_sym(ast.children.at(0));
        Sym s2 = analyze_sym(ast.children.at(2));
        res.op = ",";
        res.children = {s1, s2};
        //std::cout << "got (a,b)" << std::endl;
    }
    else if(ast.tok.type == "braced_sym_list"){
        /// {"\\(", "sym_list", "\\)"}
        assert_arity(ast, 3,3);
        res = analyze_sym(ast.children.at(1));
        //std::cout << "got ((...))" << std::endl;
    }
    else{
        std::stringstream ss;
        ss << "dunno how to analyze symbol: " << ast.tok;
        std::cout << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
    return res;
}
