#include <any>
#include <optional>
#include <string>
#include <vector>
/// the basic value

class GroundVal{
    public:
    std::string type;
    std::any val;
};

class Sym{
    public:
    enum eType{ERROR,OP, IDENT, VAL} sym_type;
    
    Sym(std::string name="",std::string ret_type="",eType type=ERROR);
    Sym(std::string name, std::string ret_type, eType t, GroundVal val);
    Sym& add_children(std::vector<Sym> new_children);

    std::string name;
    std::string ret_type;
    std::optional<GroundVal> ground;
    std::vector<Sym> children;
};

namespace GroundVals{
    GroundVal _void();
    GroundVal _int(int n);
    GroundVal _float(float f);
    GroundVal _char(char c);
    GroundVal _string(std::string S);
};

namespace Syms{
    /// values
    Sym _ident(std::string name);
    Sym _int(int num);
    Sym _float(float num);
    Sym _char(char c);
    Sym _string(std::string str);
    Sym _val(std::string type, std::any val);

    /// control flow
    Sym _return(Sym res);
    Sym _if(Sym cond, Sym if_stmt, std::vector<Sym> else_stmts={});
    Sym _while(Sym cond, Sym stmt);
    Sym _for(Sym list, Sym stmt);
    Sym _func(Sym ident, std::vector<Sym> args, Sym stmt);
    Sym _block(std::vector<Sym> stmts);

    /// types and variables
    Sym _decl(Sym A);
    Sym _assign_sym(Sym A, Sym B);
    Sym _assign_eval(Sym A, Sym B);
    /// operators
    /// - arithmeic
    Sym _add(Sym A, Sym B); // a + b
    Sym _sub(Sym A, Sym B); // a - b
    Sym _mul(Sym A, Sym B); // a * b
    Sym _div(Sym A, Sym B); // a / b
    Sym _pow(Sym A, Sym B); // a ^ b
    Sym _mod(Sym A, Sym B); // a % b
    Sym _idx(Sym A, Sym B); // a[b]
    /// - control flow
    Sym _call(Sym A, std::vector<Sym> args);
    /// - evaluation order
    Sym _pure_eval(Sym A);
    Sym _effect_eval(Sym A);
    Sym _defer(Sym A);
};
