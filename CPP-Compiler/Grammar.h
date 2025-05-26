//
// Created by felixg on 25/05/2025.
//

#ifndef GRAMMAR_H
#define GRAMMAR_H
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <sstream>

class Symbol {
private:
    std::string nombre;
    //bool esTerminal_;
public:
    Symbol() = default;
    explicit Symbol(const std::string& n) : nombre(n) {}
    std::string getNombre() const {
        return nombre;
    }
    // ---------- SI ESQUE SE MODIFICA LA GRAMATICA SE CAMBIARA ESTOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
    std::unordered_set<std::string> terminales = {"$", "ID", "if", "replay", "print", "export", "append", "remove", ":", "=", "(", ")", "then", "end", "in", "INT_LITERAL", "STRING_LITERAL", ";", ",", "Video", "Playlist", "Int", "Time", "Bool", ">", "<", "eq", "neq", "+", "-", ">>", "++", "TIME_LITERAL", "load", "duration", "speed", "WResolution", "HResolution", "Format", "clip", "[", "]"};

    bool esTerminal() const {
        return terminales.contains(nombre);
    }
    bool operator==(const Symbol& other) const {
        return nombre == other.nombre;
    }
    bool operator<(const Symbol& other) const {
        return nombre < other.nombre;
    }
};

class Production {
private:
    int id;
    Symbol ladoIzquierdo;
    std::vector<Symbol> ladoDerecho;
public:
    Production() = default;
    Production(int id_, const Symbol& izq, const std::vector<Symbol>& der)
        : id(id_), ladoIzquierdo(izq), ladoDerecho(der) {}
    int getId() const {
        return id;
    }

    int getID() const {
        return id;
    }
    Symbol getLadoIzquierdo() const {
        return ladoIzquierdo;
    }
    std::vector<Symbol> getLadoDerecho() const {
        return ladoDerecho;
    }

    void print() const;
};

struct NonTerminalFeatures {
    bool nullable = false;
    std::set<std::string> firstSet;
    std::set<std::string> followSet;
};

class Grammar {
private:
    std::vector<Production> producciones;
    std::map<std::string, NonTerminalFeatures> first_follow_Table;
public:
    std::unordered_map<std::string, std::unordered_map<std::string, int>> parsingTable;
    std::map<int, Production> mapProducciones;

    void cargarFirstFollowTable(const std::string& pathFirst, const std::string& pathFollow, const std::string& pathNullable);
    void cargarDesdeArchivo(const std::string& nombreArchivo);
    const std::vector<Production>& getProductiones() const {
        return producciones;
    }
    void imprimirFirstFollowTable() const;
    // REVISAR MATUSCULAS
    std::unordered_set<std::string> noTerminales = {"Program", "StmtList", "StmtList'", "Stmt", "StmtDA", "StmtDA'", "Decl", "Assign", "IfStmt", "ForStmt", "IterSource", "PrintStmt", "ExportStmt", "FunStmt", "Type", "Expr", "CondExpr", "RelExpr", "RelExpr'", "AddExpr", "AddExpr'", "TransExpr", "TransExpr'", "ConcatExpr", "ConcatExpr'", "PrimExpr", "FuncCall", "ListLit", "IDList", "IDList'", "CompOp", "Number"};
    std::unordered_set<std::string> terminales = {"$", "ID", "if", "replay", "print", "export", "append", "remove", ":", "=", "(", ")", "then", "end", "in", "INT_LITERAL", "STRING_LITERAL", ";", ",", "Video", "Playlist", "Int", "Time", "Bool", ">", "<", "eq", "neq", "+", "-", ">>", "++", "TIME_LITERAL", "load", "duration", "speed", "WResolution", "HResolution", "Format", "clip", "[", "]"};
    std::map<std::string, std::vector<std::pair<std::string, int>>> firstWithProd;
    void InitFirstWithProd();
    void buildParsingTable();

private:
    std::string trim(const std::string& s) const;
};

#endif //GRAMMAR_H