#include "util/cli_options.h"
#include <iostream>
#include <sstream>
using namespace std;



cli_options opt;

cli_options::cli_options()
{}

bool isOpt(const char *str, std::string S_short, std::string S_long){
    return (str && ((S_short == str) || (S_long == str)));
}

//template<typename T> bool readval(T &x, std::string name, const char *word2){
//    std::stringstream ss(word2);
//    if(ss >> x){
//        std::cout << "selected " << name << " [" << x << "]" << std::endl;
//        return true;
//    }else{
//        std::cout << "bad arg: [" << word2 << "]" << std::endl;
//        return false;
//    }
//}

//bool multiplechoice(
//    std::string &x, 
//    std::vector<std::string> choices, 
//    std::string name, 
//    const char *word2)
//{
//    for(const auto& choice:choices){
//        if(choice == word2){
//            x = choice;
//            std::cout << "selected " << name << " [" << choice << "]" << std::endl;
//            return true;
//        }
//    }
//    return false;
//}

//template<typename T> void setflag(T &x, std::string name){
//    x = true;
//    std::cout << name << " activated" << std::endl;
//}

//bool ends_with(std::string value, std::string ending)
//{
//    if(ending.size() > value.size()) return false;
//    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
//}

//void badarg(const char *str){
//    std::cout << "bad arg: [" << str << "]" << std::endl;
//}

std::istream& operator>>(std::istream &stream, cli_opt &opt){
  auto vui = std::get_if<unsigned int>(&opt.val);
  auto vd  = std::get_if<double>(&opt.val);
  auto vb  = std::get_if<bool>(&opt.val);
  auto vs  = std::get_if<std::string>(&opt.val);

  if(vui){stream >> *vui;}
  if(vd){stream >> *vd;}
  if(vb){stream >> *vb;}
  if(vs){stream >> *vs;}

  return stream;
}


bool cli_options::parseOptions(int argc, char **argv){
    if(argc == 1){
        return 0;
    }
    int I  = 1;
    while(argv[I]){
        const char *word1 = argv[I];
        const char *word2 = word1? argv[I+1] : 0;
        
        bool found = false;
        for(auto& [k,v] : options){
            //std::cout << "word [" << word1 << "]: check opt [" 
            //    << v.opt_short << ", " << v.opt_long << "]" 
            //    << std::endl;
            if(isOpt(word1, v.opt_short, v.opt_long)){
                found = true;
                if(std::get_if<bool>(&v.val) != nullptr){
                    v.val = true;
                    v.assigned = true;
                }else{
                    I++; /// right now, all but bool need an argument
                    if(!word2){word2 = "";}
                    std::stringstream ss(word2);
                    if(ss >> v){
                        v.assigned = true;
                    }else{
                        std::stringstream ss;
                        ss << "could not parse value [" << word2 << "]"
                        << " for option (" << v.opt_short << ", " << v.opt_long << ")";
                        throw std::runtime_error(ss.str());
                    }
                }
            }
        }
        if(!found){
            std::stringstream ss;
            ss << "unknown option: [" << word1 << "]";
            throw std::runtime_error(ss.str());
        }
        I++;
    }
    return 1;
}

cli_opt& cli_options::operator[](std::string name){return options.at(name);}
bool cli_options::has(std::string name){
    return options.at(name).assigned;
}