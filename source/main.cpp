#include <iostream>
#include "util/cli_options.h"
#include "parser/tokenizer.h"
#include "parser/parser.h"
#include "util/output.h"
#include "parser/ast_fixer.h"
#include <cassert>

void print_help();
void print_argc_argv(int argc, char** argv);
void compile();

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

    compile();

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

typedef unsigned int uint;




struct cfg_compile{
    std::string file_in;
    std::string text_in;
    bool do_normal_compile=false;
    bool do_stage_tokens=false;
    bool do_stage_parse=false;
    bool do_output_tokens=false;
    bool do_output_parse=false;
    bool do_test_parse=false;
    std::string json_out_toks;
    std::string json_out_ast;
};

class Compiler{
    public:
    Tokenizer tokenizer;
    Parser parser;
    Parser::AST ast;
    std::optional<Parser::AST> parse_template;

    void tokenize(cfg_compile cfg){
        tokenizer.reset();
        // prepare input
        assert(cfg.file_in.length() || cfg.text_in.length());
        if(cfg.file_in.length()){tokenizer.open_file(cfg.file_in);}
        else                    {tokenizer.set_text(cfg.text_in);}
        // load input
        tokenizer.load();
        // optional intermediate outputs
        if(cfg.do_output_tokens){
            auto toks_out = out.tokens_to_json(tokenizer.tokens);
            out.write_file(cfg.json_out_toks, toks_out);
        }
        // remove comments etc
        tokenizer.tokens = tokenizer.erase_token_type(tokenizer.tokens, {"COMMENT", "SP", "NL"});
    }
    void parse(cfg_compile cfg){
        /// go from Tokens to AST
        ast = parser.process(tokenizer.tokens, parse_template);
        /// optional intermediate outputs
        if(cfg.do_output_parse){
            auto ast_pretty = ast_unroll_lists(ast);
            ast_pretty = ast_merge_singles(ast_pretty);
            calc_NT_positions(ast_pretty);
            auto ast_out = out.ast_to_json(ast_pretty);
            out.write_file(cfg.json_out_ast, ast_out);
        }
    }
    void run(cfg_compile cfg){
        if(cfg.do_test_parse){                  /// run tests
            test_parse(cfg);
        }
        if(cfg.do_normal_compile){               /// compile cli input file
            basic_run(cfg);
        }
    }
    void basic_run(cfg_compile cfg){
        if(!(cfg.do_stage_tokens)){return;}
        tokenize(cfg);

        if(!(cfg.do_stage_parse)){return;}
        parse(cfg);
    }
    void test_parse(cfg_compile cfg){
        auto cfg2 = cfg;
        cfg2.file_in = "";
        cfg2.json_out_toks = "data_out/test_parse_tokens.json";
        cfg2.json_out_ast  = "data_out/test_parse_ast.json"; 
        for(auto test:Parser::tests){
            std::cout << "test [" << test.first << "]...";
            cfg2.text_in = test.first;
            parse_template = test.second;
            basic_run(cfg2);
            parse_template = std::nullopt;
            std::cout << "OK\n";
        }
    }
};


void compile(){
    cfg_compile cfg;
    cfg.file_in = (std::string&)opt["filename_in"];
    cfg.do_stage_tokens = true;
    cfg.do_stage_parse = true;
    cfg.do_output_tokens = true;
    cfg.do_output_parse = true;
    cfg.do_normal_compile = true;
    cfg.do_test_parse = true;
    cfg.json_out_toks = "data_out/tokens.json";
    cfg.json_out_ast  = "data_out/ast.json";

    Compiler compiler;
    compiler.run(cfg);
}
