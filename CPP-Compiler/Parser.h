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
#include <list>
#include <stack>
#include "Grammar.h" // Class Symbol, Production, NonTerminalFeatures and Grammar
#include "Scanner.h"

std::string removeQuotes(std::string input) ;

struct TreeNode
{
    int nodeId;
    Symbol symbol;
    const TreeNode* parentNode;
    list<TreeNode*> children;
    std::string nullableValue;
    bool isVar() const
    {
        return symbol.getNombre() == "INT_LITERAL"
        || symbol.getNombre() == "STRING_LITERAL"
        || symbol.getNombre() == "TIME_LITERAL"
        || symbol.getNombre() == "ID";
    }

    string completeValue() const
    {
        return symbol.getNombre() + " , " + nullableValue;
    }
    TreeNode(const std::string& symbol_name, const TreeNode* parent, const int id) : nodeId(id), symbol(Symbol(symbol_name)), parentNode(parent) {}

    // Destructor recursivo
    ~TreeNode() {
        for (TreeNode* child : children) {
            delete child;  // Libera cada hijo (y recursivamente sus hijos)
        }
    }

    // Eliminar copias no deseadas
    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;
};

class Parser {
public:
    Grammar Gram;
    Scanner Scan;
    std::stack<string> stackParser;
    std::stack<TreeNode*> treeNodesStack;
    std::string input;
    int errorCount = 0;
    Parser(const Grammar& grammar, const Scanner& scanner) : Gram(grammar), Scan(scanner) {
        Scan.reset();
        stackParser.push("$");
        /// Añandimos la produccion incial (no terminal)
        stackParser.push(Gram.mapProducciones[0].getLadoIzquierdo().getNombre());
    }
    void parsing();
    int getErrorCount() const {
        return errorCount;
    }
    void printStack(const std::stack<std::string>& stack);
private:
    void exportTreeToFile(const TreeNode* root, const std::string& filename) const;
};


#endif // PARSER_H