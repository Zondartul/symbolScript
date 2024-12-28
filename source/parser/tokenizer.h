#pragma once
#include <string>
#include <regex>
#include <set>
#include <map>

class miniTokenizer{
    public:
    struct token{
        std::string type;
        std::string text;
    };
    struct token_info{
        std::regex reg;
        std::string type;
    };
    static std::vector<token> tokenize(const std::string &S, std::vector<token_info> tok_dict);
};

class FileStore{
    public:
    std::map<std::string, std::string> files;
    std::string &open_file(std::string filename);
};

extern FileStore fileStore;

class Tokenizer{
    public:
    struct tok_pos{ /// coordinates that point to some character in some text file
        std::string filename;
        int char_idx = -1;
        int line = -1; // -1 means uninitialized
        int col = -1;
        bool operator<(const tok_pos& other) const;
    };

    struct token{
        std::string type;
        std::string text;
        tok_pos pos1; //from
        tok_pos pos2; //to
        bool operator==(const token& other) const;
        bool operator<(const token& other) const;
    };

    Tokenizer();
    ~Tokenizer();
    void reset();
    void open_file(std::string filename);
    void set_text(std::string text);
    void load();

    std::string file;
    std::vector<token> tokens;
    static std::string cur_filename; ///so like, why even make things static if we gonna do this anyway?

    using v_minitoken_infos = std::vector<miniTokenizer::token_info>;
    static v_minitoken_infos tok_dict;
    /// helper functions

    static std::vector<token> annotate_positions(const std::vector<miniTokenizer::token>& tokens);
    static std::vector<token> erase_token_type(const std::vector<token>& tokens, std::set<std::string> erased_types);
    static void print_tokens(std::vector<token> tokens);
};

bool operator==(const Tokenizer::tok_pos& pos1, const Tokenizer::tok_pos& pos2);