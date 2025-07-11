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
#include <list>
#include <algorithm> // Para std::find y std::min_element
#include <memory>    // Para std::unique_ptr o std::shared_ptr si se usara gestión RAII
#include <limits>

std::string removeQuotes(std::string input) ;
const std::vector<std::string> AST_ELEMENTS_PRIORITY = {
    "Program",
    //"Stmt",
    //"Decl",           // Declaración (más específica que StmtDA)
    //"Assign",         // Asignación (más específica que StmtDA)
    // 1. Nodos de Estructura Principal y Contenedores (más alta prioridad)
    // 4. Nodos que representan llamadas a funciones específicas o estructuras de datos compuestas
    //"FuncCall",       // Representa una llamada a función (load, duration, etc.)
    "ListLit",        // Literal de lista (ej. [IDList])
    "IDList",         // Lista de identificadores (dentro de ListLit)
    "=",
    "if",
    "replay",
    "in",
    "append",
    "remove",
    "load",
    "duration",
    "speed",
    "WResolution",
    "HResolution",
    "Format",
    "clip",
    "print",
    "export",


    //"StmtList",       // Secuencia de sentencias
    "Stmt",           // Sentencia individual

    // 2. Nodos de Control de Flujo y Declaraciones/Asignaciones de alto nivel
    /*
    "IfStmt",         // Sentencia if
    "ForStmt",        // Sentencia for
    "StmtDA",         // Declaración o asignación (agrupa Decl/Assign)

    "FunStmt",        // Llamada a función (si es el nodo que agrupa la llamada, ej. append, remove)
    "PrintStmt",      // Sentencia print
    "ExportStmt",     // Sentencia export

    // 3. Nodos de Expresión (Orden por precedencia semántica de operaciones, si aplica)
    "Expr",           // Expresión general
    */
    /*
    "CondExpr",       // Expresión condicional
    "RelExpr",        // Expresión relacional
    "AddExpr",        // Expresión aditiva
    "TransExpr",      // Expresión de transformación
    "ConcatExpr",     // Expresión de concatenación
*/


    "+",
    "-",
    ">>",
    "++",
    ">",
    "<",
    "eq",             // Operador de igualdad (==)
    "neq",            // Operador de desigualdad (!=)


    // 7. Identificadores y Literales (las "hojas" del AST, los valores concretos)
    "ID",             // Identificador (ej., "miVariable")
    "INT_LITERAL",    // Literal entero (ej., "34")
    "TIME_LITERAL",   // Literal de tiempo
    "STRING_LITERAL", // Literal de cadena

    // 8. Palabras clave funcionales (terminales que activan una acción o son parte de una estructura)
    // Se han movido hacia el final porque a menudo son hijos directos de nodos de sentencia o expresión,
    // y no "suben" a tomar el lugar de un padre de nivel superior.

};



struct TreeNode
{
    bool flag_comp =false;
    int deep = 0;
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
TreeNode* reduceParseTreeToAST(TreeNode* node);

class Parser {
public:
    Grammar Gram;
    Scanner Scan;
    std::unordered_map<string, string> varTable;
    std::stack<string> stackParser;
    std::stack<TreeNode*> treeNodesStack;
    std::string input;
    int errorCount = 0;
    unordered_set<string> error_sync = {";", "$", "end"};
    unordered_set<string> list_sync = {"Program", "StmtList'", "Stmt"};
    Parser(const Grammar& grammar, const Scanner& scanner) : Gram(grammar), Scan(scanner) {
        Scan.reset();
        stackParser.push("$");
        /// Añandimos la produccion incial (no terminal)
        stackParser.push(Gram.mapProducciones[0].getLadoIzquierdo().getNombre());
    }
    TreeNode* parsing();
    int getErrorCount() const {
        return errorCount;
    }
    void printStack(const std::stack<std::string>& stack);
    void printVarTable();

private:
    void exportTreeToFile(const TreeNode* root, const std::string& filename) const;

};


#endif // PARSER_H