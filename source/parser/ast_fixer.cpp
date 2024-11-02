#include "parser/ast_fixer.h"
#include <iostream>
/*  from LazyCompiler: yaccin.y
    //		B = B C | C
	//beceomes
	//		B = C C C C C
	void ast_unroll_lists_helper(ast_node *N2){
		//ast_node *N2 = ast_get_node(nodeID2);
		if(!N2->children.size){return;}
		ast_node *N3 = ast_get_child(N2,0);
		if(N2->token.type != N3->token.type){return;}
		ast_unroll_lists_helper(ast_get_child(N2, 0));
		m(N2->children,erase,0);
		int i;
		for(i = 0; i < N3->children.size; i++){
			//m(N2->children,push_back,ast_get_child_id(N3,i));
			m(N2->children,push_back,ast_get_child(N3,i));
		}
	}
	//turns left-recursive lists into non-recursive ones
	//so	A = B | empty
	//		B = B C | C
	//becomes	A = C C C C C C | empty
	
	void ast_unroll_lists(ast_node *N1){
		//ast_node *N1 = ast_get_node(nodeID);
		printf("unroll [%s] ",N1->token.type);
		if(!N1->children.size){return;}
		ast_node *N2 = ast_get_child(N1, 0);
		if(!N2->children.size){return;}
		//ast_unroll_lists_helper(ast_get_child_id(N1, 0));
		ast_unroll_lists_helper(ast_get_child(N1,0));
		m(N1->children,erase,0);
		int i;
		for(i = 0; i < N2->children.size; i++){
			//m(N1->children,push_front,ast_get_child_id(N2,i));
			m(N1->children,push_front,ast_get_child(N2,i));
		}
		printf(" unroll done\n");
	}
*/

using AST = MiniParser::AST;
using Token = Tokenizer::token;
//void dbg_aulh_filter(const AST &N){
//    if(N.tok.type == "statement_list"){
//        if(N.children.size() == 2){
//            if((N.children.at(0).tok.type == "statement_list")
//                && (N.children.at(1).tok.type == "statement_list")){
//                    //std::cout << "debug breakpoint" << std::endl;
//                }
//        }
//    }
//}

AST ast_unroll_lists_helper(AST N2){
    //dbg_aulh_filter(N2);
    if(!N2.children.size()){return N2;}
    AST N3 = N2.children.at(0);
    if(N2.tok.type == N3.tok.type){
        N3 = ast_unroll_lists_helper(N3);
        N2.children.erase(N2.children.begin());
        for(auto ch:N3.children){
            N2.children.push_back(ast_unroll_lists_helper(ch));
        }
    }else{
        for(auto &ch:N2.children){
            ch = ast_unroll_lists_helper(ch); // bypass
        }
    }
    return N2;
}

AST ast_unroll_lists(AST N1){
    //std::cout << "unroll [" << N1.tok.type << "]" << std::endl;
    if(!N1.children.size()){return N1;}
    AST N2 = N1.children.at(0);
    if(!N2.children.size()){return N1;}
    N2 = ast_unroll_lists_helper(N2);
    N1.children.erase(N1.children.begin());
    for(auto ch: N2.children){
        N1.children.insert(N1.children.begin(),ch);
    }
    return N1;
}

std::string merge_with_delim(std::string Sa, std::string Sb, std::string delim){
    return Sa + ((!Sa.empty() && !Sb.empty())? delim : "") + Sb;
}

Token ast_merge_token(Token A, Token B){
    Token res = A;
    res.text = merge_with_delim(A.text, B.text, ".");
    res.type = merge_with_delim(A.type, B.type, ".");
    return res;
}

AST ast_merge_singles(AST N){
    //std::cout << "merge singles" << std::endl;
    if(!N.children.size()){return N;}
    if(N.children.size() == 1){
        auto tok_pr = N.tok;
        auto tok_ch = N.children.at(0).tok;
        N = N.children.at(0);
        N.tok = ast_merge_token(tok_pr, tok_ch);
    }else{
        for(auto &ch:N.children){
            ch = ast_merge_singles(ch);
        }
    }
    return N;
}