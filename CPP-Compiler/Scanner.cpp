#include "Scanner.h"
#include <iostream>

char Scanner::getchar()
{
    if (index >= source.size()) return '\0';
    char c = source[index++];
    if (c == '\n') { line++; col = 1; }
    else col++;
    return c;
}

char Scanner::peekchar() const
{
    if (index >= source.size()) return '\0';
    return source[index];
}

bool Scanner::isBool(const string& s)
{
    return s == "true" || s == "false";
}

bool Scanner::isInteger(const string& s)
{
    if (s.empty()) return false;
    for (char c : s) if (!is_digit(c)) return false;
    return true;
}

bool Scanner::isTimeFormat(const string& s)
{
    if (s.size() != 8) return false;
    return is_digit(s[0]) && is_digit(s[1]) &&
        s[2] == ':' &&
        is_digit(s[3]) && is_digit(s[4]) &&
        s[5] == ':' &&
        is_digit(s[6]) && is_digit(s[7]);
}

Token Scanner::gettoken()
{
    while (is_space(peekchar())) getchar();
    int tokenLine = line;
    int tokenCol = col;
    char c = peekchar();

    if (c == '\0') {
        return { EOP, "$", "EOP", tokenLine, tokenCol };
    }

    if (c == '%') {
        string comment;
        while (peekchar() != '\n' && peekchar() != '\0') comment += getchar();
        //return { COMMENT, comment, "COMMENT", tokenLine, tokenCol };

        while (is_space(peekchar())) getchar();
        c = peekchar();
        if (c == '\0') {
            return { EOP, "$", "EOP", tokenLine, tokenCol };
        }

    }

    if (is_alpha(c)) {
        string word;
        while (is_alnum(peekchar())) word += getchar();
        if (keywordTypes.count(word))
            return { KEYWORD, word, keywordTypes[word], tokenLine, tokenCol };
        if (isBool(word))
            return { BOOL, word, word == "true" ? "BOOL_TRUE" : "BOOL_FALSE", tokenLine, tokenCol };
        return { ID, word, "ID", tokenLine, tokenCol };
    }

    if (is_digit(c)) {
        string num;
        int colons = 0;
        while (is_digit(peekchar()) || peekchar() == ':') {
            if (peekchar() == ':') colons++;
            num += getchar();
        }
        if (colons == 2 && isTimeFormat(num)) return { TIME, num, "TIME", tokenLine, tokenCol };
        if (colons == 0 && isInteger(num)) return { INT, num, "INT", tokenLine, tokenCol };
        errorCount++;
        return { ERROR, num, "INVALID_TIME_OR_INT", tokenLine, tokenCol };
    }

    if (c == '"') {
        string str;
        str += getchar();
        while (peekchar() != '"' && peekchar() != '\0' && peekchar() != '\n') str += getchar();
        if (peekchar() == '"') {
            str += getchar();
            return { STRING, str, "STRING", tokenLine, tokenCol };
        }
        errorCount++;
        return { ERROR, str, "UNCLOSED_STRING", tokenLine, tokenCol };
    }

    string twoChar;
    twoChar += getchar();
    twoChar += peekchar();
    if (doubleCharOps.count(twoChar)) {
        getchar();
        return { OPERATOR, twoChar, operatorTypes[twoChar], tokenLine, tokenCol };
    }

    if (operatorTypes.count(string(1, twoChar[0]))) {
        return { OPERATOR, string(1, twoChar[0]), operatorTypes[string(1, twoChar[0])], tokenLine, tokenCol };
    }

    errorCount++;
    return { ERROR, string(1, twoChar[0]), "INVALID_CHAR", tokenLine, tokenCol };
}

void Scanner::printToken(const Token& token)
{
    static const char* TokenNames[] = {
        "ID", "INT", "TIME", "STRING", "BOOL", "KEYWORD",
        "OPERATOR", "DELIMITER", "COMMENT", "ERROR", "EOP"
    };

    if(TokenNames[token.type] == "ERROR")
    {
        cout<<TokenNames[token.type]<< " [ " << token.value << " ] (" << token.specificType
            << ") found at (" << token.line << ":" << token.col << ")\n";
    }
    else {

        cout << "S - " << " [ " << token.value << " ] (" << token.specificType
            << ") found at (" << token.line << ":" << token.col << ")\n";
    }
}

int Scanner::getErrorCount() const
{
    return errorCount;
}

void Scanner::reset() {
    index = 0;
    line = 1;
    col = 1;
    errorCount = 0;
}
