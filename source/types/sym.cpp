#include "types/sym.h"


Sym::Sym(std::string name, std::string ret_type, eType sym_type):
    name(name),ret_type(ret_type),sym_type(sym_type){}
Sym::Sym(std::string name, std::string ret_type, eType sym_type, GroundVal val):
    name(name),ret_type(ret_type),sym_type(sym_type),ground(val){}
Sym& Sym::add_children(std::vector<Sym> new_children){
    children.insert(children.end(), new_children.begin(), new_children.end());
    return *this;
}

namespace GroundVals{
    GroundVal _void()               {GroundVal res; res.type = "void"; return res;}
    GroundVal _int(int n)           {return {"int", n};}
    GroundVal _float(float f)       {return {"float",f};}
    GroundVal _char(char c)         {return {"char",c};}
    GroundVal _string(std::string S){return {"string",S};}
    GroundVal _val(std::string type, std::any val){return {type,val};}
};

namespace Syms{
    /// values
    Sym _ident(std::string name)    {return Sym(name,   "",     Sym::IDENT);}
    Sym _int(int num)               {return Sym("",     "int",  Sym::VAL,   GroundVals::_int(num));}
    Sym _float(float num)           {return Sym("",     "float",Sym::VAL,   GroundVals::_float(num));}
    Sym _char(char c)               {return Sym("",     "char", Sym::VAL,   GroundVals::_char(c));}
    Sym _string(std::string S)      {return Sym("",     "string",Sym::VAL,  GroundVals::_string(S));}
    Sym _val(std::string type, std::any val){return Sym("", type, Sym::VAL, GroundVals::_val(type,val));}

    /// control flow
    Sym _return(Sym res){
        return Sym("return", res.ret_type, Sym::OP).add_children({res});
    }
    Sym _if(Sym cond, Sym if_stmt, std::vector<Sym> else_stmts){
        else_stmts.insert(else_stmts.begin(), if_stmt);
        return Sym("if",if_stmt.ret_type,Sym::OP).add_children(else_stmts);
    }
    Sym _while(Sym cond, Sym stmt){
        return Sym("while",stmt.ret_type,Sym::OP).add_children({cond, stmt});
    }
    Sym _for(Sym list, Sym stmt){
        return Sym("for",stmt.ret_type,Sym::OP).add_children({list,stmt});
    }
    Sym _func_arglist(std::vector<Sym> args){
        return Sym("(,)","",Sym::OP).add_children(args);
    }
    Sym _func(Sym ident, std::vector<Sym> args, Sym stmt){
        return Sym("func",stmt.ret_type,Sym::OP).add_children({Syms::_func_arglist(args), stmt});
    }
    Sym _block(std::vector<Sym> stmts){
        return Sym("{;}","BLOCK_RET_TYPE",Sym::OP).add_children(stmts);
    }

    /// types and variables
    Sym _decl(Sym A){
        return Sym("decl",A.ret_type,Sym::OP).add_children({A});
    }
    Sym _assign_sym(Sym A, Sym B){
        return Sym("=",B.ret_type,Sym::OP).add_children({A,B});
    }
    Sym _assign_eval(Sym A, Sym B){
        return Sym(":=",B.ret_type,Sym::OP).add_children({A,B});
    }
    /// operators
    /// - arithmeic
    Sym _add(Sym A, Sym B){return Sym("+",A.ret_type,Sym::OP).add_children({A,B});} // a + b
    Sym _sub(Sym A, Sym B){return Sym("-",A.ret_type,Sym::OP).add_children({A,B});} // a - b
    Sym _mul(Sym A, Sym B){return Sym("*",A.ret_type,Sym::OP).add_children({A,B});} // a * b
    Sym _div(Sym A, Sym B){return Sym("/",A.ret_type,Sym::OP).add_children({A,B});} // a / b
    Sym _pow(Sym A, Sym B){return Sym("^",A.ret_type,Sym::OP).add_children({A,B});} // a ^ b
    Sym _mod(Sym A, Sym B){return Sym("%",A.ret_type,Sym::OP).add_children({A,B});} // a % b
    Sym _idx(Sym A, Sym B){return Sym("[]",A.ret_type,Sym::OP).add_children({A,B});} // a[b]
    /// - control flow
    Sym _call(Sym A, std::vector<Sym> args){
        args.insert(args.begin(),A);
        return Sym("call",A.ret_type,Sym::OP).add_children(args);
    }
    /// - evaluation order
    Sym _pure_eval(Sym A){return Sym("!x",A.ret_type,Sym::OP).add_children({A});}
    Sym _effect_eval(Sym A){return Sym("x!",A.ret_type,Sym::OP).add_children({A});}
    Sym _defer(Sym A){return Sym("&x",A.ret_type).add_children({A});}
};
