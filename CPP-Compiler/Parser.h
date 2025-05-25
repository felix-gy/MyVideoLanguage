#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <stack>
#include "Grammar.h" // Class Symbol, Production, NonTerminalFeatures and Grammar
#include "Scanner.h"

struct ParseNodeTree
{
    string Symbol_name = "";
    string Head_name = "";
    ParseNodeTree(const std::string& symbol_name, const std::string& head_name) : Symbol_name(symbol_name), Head_name(head_name) {}
};

class Parser {
public:
    Grammar Gram;
    Scanner Scan;
    std::stack<string> stackParser;

    std::string input;
    int errorCount = 0;
    Parser(const Grammar& grammar, const Scanner& scanner) : Gram(grammar), Scan(scanner) {
        Scan.reset();
        stackParser.push("$");
        /// Añandimos la produccion incial (no terminal)
        stackParser.push(Gram.mapProducciones[0].getLadoIzquierdo().getNombre());
    }
    void parse();
    int getErrorCount() const {
        return errorCount;
    }
    void printStack(const std::stack<std::string>& stack);
private:

};


#endif // PARSER_H