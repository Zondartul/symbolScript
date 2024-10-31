#pragma once
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <map>

/// supported CLI option types:
/// - bool - option is either specified (true) or not specified.
///        - future: use 'no-' prefix to specify false.
/// - uint, double - a numeric value should be provided and parsed.
/// - string - a string is parsed (either a space-delimited word, or (future) quoted string)
using cli_opt_val = typename std::variant<unsigned int, double, bool, std::string>;

struct cli_opt{
  std::string opt_long;  /// --blah
  std::string opt_short; /// -b
  cli_opt_val val;
  bool assigned;         /// whether the option was specified in the CLI call
  template<typename T> operator T&(){
    return get<T>(val);
  }
};



class cli_options{
public:
  cli_options();

  std::map<std::string, cli_opt> options;

  bool has(std::string name);            /// returns true if option is specified
  cli_opt& operator[](std::string name); /// returns the value of the option
  bool parseOptions(int argc, char **argv);
  void printOptions();                   /// debug info about supplied options
};

extern cli_options opt;