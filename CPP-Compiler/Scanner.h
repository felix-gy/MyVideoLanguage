//
// Created by felixg on 12/04/2025.
//

#ifndef SCANNER_H
#define SCANNER_H
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include "Token.h"

using namespace std;
class Scanner {
private:
    string source;
    size_t index = 0;
    int line = 1;
    int col = 1;
    int errorCount = 0;

    unordered_map<string, string> keywordTypes = {
        {"if", "IF_KEY"}, {"then", "THEN_KEY"}, {"end", "END_KEY"},
        {"replay", "LOOP_KEY"}, {"in", "IN_KEY"},
        {"load", "LOAD_KEY"}, {"export", "EXPORT_KEY"},
        {"duration", "DURATION_KEY"}, {"speed", "SPEED_KEY"},
        {"clip", "CLIP_KEY"}, {"print", "PRINT_KEY"},
        {"leer_video", "READ_KEY"}, {"cargar_video", "LOADVID_KEY"},
        {"Video", "TYPE_VIDEO"}, {"PlayList", "TYPE_PLAYLIST"},
        {"Int", "TYPE_INT"}, {"Time", "TYPE_TIME"},
        {"String", "TYPE_STRING"}, {"Bool", "TYPE_BOOL"},
        {"eq", "OP_EQ"}, {"neq", "OP_NEQ"},
        {"true", "BOOL_TRUE"}, {"false", "BOOL_FALSE"},
        {"WResolution", "PIXEL_ANCHO"}, {"HResolution","PIXEL_ALTO"},
        {"add", "ADD_IN_LIST"}, {"remove", "REMOVE_OF_LIST"}
    };

    unordered_map<string, string> operatorTypes = {
        {"+", "ADD_OP"}, {"-", "SUB_OP"},
        {"*", "MUL_OP"}, {"/", "DIV_OP"},
        {"=", "ASSIGN_OP"}, {":", "TYPE_ASSIGN"},
        {">", "GT_OP"}, {"<", "LT_OP"},
        {">>", "SHIFT_RIGHT_OP"}, {"<<", "SHIFT_LEFT_OP"},
        {",", "COMMA"}, {"(", "OPEN_PAR"}, {")", "CLOSE_PAR"},
        {"[", "OPEN_BRACKET"}, {"]", "CLOSE_BRACKET"},
        {"\"", "QUOTE"}, {";", "CLOSE"}
    };
    // Agregar el ++ y el punto y coma , cambia append y remove
    unordered_set<string> doubleCharOps = { ">>"};
    unordered_set<char> singleCharOps = {
        '+', '-', '*', '/', '=', '<', '>', ':', ',', '(', ')', '[', ']', '"',';'
    };

    inline bool is_digit(char c) { return isdigit(static_cast<unsigned char>(c)); }
    inline bool is_alpha(char c) { return isalpha(static_cast<unsigned char>(c)); }
    inline bool is_alnum(char c) { return isalnum(static_cast<unsigned char>(c)); }
    inline bool is_space(char c) { return isspace(static_cast<unsigned char>(c)); }

    char getchar();
    char peekchar() const;

    bool isBool(const string& s);
    bool isInteger(const string& s);
    bool isTimeFormat(const string& s);
public:
    Scanner(const string& src) : source(src) {}
    Token gettoken();
    void printToken(const Token& token);
    int getErrorCount() const ;
    void reset();
};

#endif // SCANNER_H
