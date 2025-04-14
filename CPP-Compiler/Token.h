#ifndef TOKEN_H
#define TOKEN_H

#include <string>
using namespace std;

enum TokenType {
    ID, INT, TIME, STRING, BOOL,
    KEYWORD, OPERATOR, DELIMITER, COMMENT, ERROR, EOP
};

struct Token {
    TokenType type;
    string value;
    string specificType; // Nuevo campo para detalles
    int line;
    int col;
};
#endif // TOKEN_H
