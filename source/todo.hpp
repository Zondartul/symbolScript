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

*/