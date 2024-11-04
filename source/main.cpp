#include <iostream>
#include "util/cli_options.h"
#include "parser/tokenizer.h"
#include "parser/parser.h"
#include "util/output.h"
#include "parser/ast_fixer.h"

void print_help();
void print_argc_argv(int argc, char** argv);

int main(int argc, char **argv){
    //print_argc_argv(argc, argv);
    opt.options = {
        {"help", {"--help", "-h", false, false}},
        {"filename_in",  {"--input", "-i", "", false}},
        {"filename_out", {"--output", "-o", "", false}},
    };
    if(!opt.parseOptions(argc, argv)){return 1;} // exit if options are bad
    opt.printOptions();
    if(opt.has("help")){print_help(); return 0;}

    std::cout << "hello world" << std::endl;
    if(opt.has("filename_in")){
        std::cout << "filaname_in: " << (std::string&)opt["filename_in"] << std::endl;
    }else{
        std::cout << "no input file specified." << std::endl;
        print_help();
        return 0;
    }

    Tokenizer tokenizer;
    tokenizer.open_file(opt["filename_in"]);
    tokenizer.load();
    //tokenizer.print_tokens(tokenizer.tokens);
    auto toks_out = out.tokens_to_json(tokenizer.tokens);
    out.write_file("data_out/tokens.json", toks_out);
    tokenizer.tokens = tokenizer.erase_token_type(tokenizer.tokens, {"COMMENT", "SP", "NL"});

    Parser parser;
    auto ast = parser.process(tokenizer.tokens);
    //MiniParser::print_AST(ast);
    auto ast_pretty = ast_unroll_lists(ast);
    ast_pretty = ast_merge_singles(ast_pretty);
    calc_NT_positions(ast_pretty);
    auto ast_out = out.ast_to_json(ast_pretty);
    out.write_file("data_out/ast.json", ast_out);

    return 0;
}

void print_help(){
    std::cout << 
    "Usage:\n"
    "symbolScript <options> [-i <filename_in>][-o <filename_out>]\n"
    "   options:\n"
    "       -h --help - this help message\n"
    "       -i --input  - path to the input file\n"
    "       -o --output - path to the output file\n" << std::endl;
}

void print_argc_argv(int argc, char** argv){
    std::cout << "argc = " << argc << std::endl;
    std::cout << "argv = " << std::endl;
    for(int i = 0; i < argc; i++){
        std::cout << "\t" << i << ": [" << argv[i] << "]" << std::endl;
    }
    std::cout << "invocation:" << std::endl;
    std::stringstream ss;
    for(int i = 0; i < argc; i++){
        ss << argv[i] << " ";
    }
    std::cout << ss.str() << std::endl;
}
