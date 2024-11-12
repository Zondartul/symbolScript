// comment
/* multiline
    comment */
A = B; // assignment
B := 5+4*3+(2*1.5); // assignment with literals

print("hello");     // function call
C = 10 + sqrt(A);   // function as part of expression

if(A) print("B");

//if(A){
//    print("B");
//}

//if(A){
//    print("B");
//}else{
//    print("C");
//}

/*
// if-block
if(A){
    print("A");
}elif(B){
    print("B");
}else{
    print("C");
}

// while-loop
while(C){
    print(C);
    C--;
}

// arrays and dicts
Arr = [];
Arr2 = [10, 20, 30];
Dict = ["derp":dorp, "beep":"boop", A:int]; /// types are first-order objects

Arr[0] << Arr2[2]; // move command
Arr[1] << 40;
Arr := Arr .* Arr2; /// replaces the symbolic value by evaluated ground value

x = {
    if(x?){ // has ground value? -> returns value if it is known, else null
        x? := x?+1; // ground value can be assigned to -> this changes the value stored in the symbol
    }else{
        x? := 0;
    }
    return x?;
};

x; // an expression by itself is usually a (variable) definition
!x; // this means 'evaluate but don't assign' - this is done at construction
x!; // this means 'evaluate-and-assign' - this normal-order
&x; // this means 'defer evaluation' - this becomes a symbolic expression when evaluated
!{  /// this means 'pure-evaluation'     
    //...
};
{

}! // this means 'impure evaluation' 
print(!x)!

/// variable structure
/// Variable -> { public: user_name, type_hint, binding 
///               private: ir_name, visibility }
/// Value structure:
/// Symbol -> { variant< symbolic_expression | ground_value >, return-type } 
///   symbolic_expression -> operator( vector<Symbol ...>)
///     operator -> <arithmetic | control_flow | function_call>
//  ground_value -> {is_null, variant<int,float,char,string,user-type>}
/// Symbol return type: 
///     return-type is specified for all symbols and determines the ground value
///     return-type is calculated when symbol is constructed from types of other symbols

/// advanced linkage stuff
//extern("C") {
//    cpc:type = "const char*";
//    print:func(str:cpc);
//    sqrt:func(N:int);
//}
*/