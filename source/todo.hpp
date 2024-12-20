/*

1. make sure the example file can be parsed/tokenized
2. output to compiler explorer (godot lazycomp viewer)
3. determine the new symbol type
4. determine new grammar
5. parse and analyze new grammar
6. code generation (maybe to LZ IR)

[IDEA 1] templated parse
        - check if statement is parsed into the reference tree
        - from template, derive rules
        -- template may or may not use identifiers for known productions
        - if earlier rule fires, insert "SHIFT"-rule and continue
        - if rule doesn't fire, it's missing?
        --- indicated rule doesn't match - fix rule
        --- template rule doesn't match any rule - create new rule
[IDEA 2] generate grammar from multiple parse templates        

[todo 3] syntax for defining functions
[IDEA 4] count how many rules could possibly apply in the future
        -- if the second stack AST can never be consumed by a future rule, we have a syntax error at the latest consumed token

*/