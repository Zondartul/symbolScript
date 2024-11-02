#include "parser/parser.h"

/// turns left-recursive lists into non-recursive ones
	/// so	A = B | empty
	///		B = B C | C
	/// becomes	A = C C C C C C | empty
Parser::AST ast_unroll_lists(Parser::AST N);

/// turns non-branching segments into single nodes
    /// so A = B = C
    //  becomes Abc
Parser::AST ast_merge_singles(Parser::AST N);