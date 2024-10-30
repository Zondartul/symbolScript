#include <iostream>
#include "util/cli_options.h"

int main(int argc, char **argv){
    opt.options = {
        {"filename_in",  {"--input", "-i", "", false}},
        {"filename_out", {"--output", "-o", "", false}},
    };
    opt.parseOptions(argc, argv);

    std::cout << "hello world" << std::endl;
    if(opt.has("filename_in")){
        std::cout << "filaname_in: " << (std::string&)opt["filename_in"] << std::endl;
    }
    return 0;
}
