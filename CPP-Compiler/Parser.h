#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <stack>

#include "Scanner.h"
// Simbolo
class Simbolo {
private:
    std::string nombre;
    //bool esTerminal_;
public:
    Simbolo() = default;
    explicit Simbolo(const std::string& n) : nombre(n) {}
    std::string getNombre() const {
        return nombre;
    }
    /*
    Simbolo(const std::string& nombre, bool esTerminal)
    : nombre(nombre), esTerminal_(esTerminal) {}
    */
    bool esTerminal() const {
        return nombre == "E" || islower(nombre[0]) || ispunct(nombre[0]) || nombre == "INT_LITERAL" || nombre == "STRING_LITERAL" || nombre == "TIME_LITERAL" || nombre == "ID";
    }
    bool operator==(const Simbolo& other) const {
        return nombre == other.nombre;
    }
    bool operator<(const Simbolo& other) const {
        return nombre < other.nombre;
    }
};

// Produccion
class Produccion {
private:
    int id;
    Simbolo ladoIzquierdo;
    std::vector<Simbolo> ladoDerecho;
public:
    Produccion() = default;
    Produccion(int id_, const Simbolo& izq, const std::vector<Simbolo>& der)
        : id(id_), ladoIzquierdo(izq), ladoDerecho(der) {}
    int getId() const {
        return id;
    }

    int getID() const {
        return id;
    }
    Simbolo getLadoIzquierdo() const {
        return ladoIzquierdo;
    }
    std::vector<Simbolo> getLadoDerecho() const {
        return ladoDerecho;
    }
};

struct NoTerminal_Atributos {
    bool nullable = false;
    std::set<std::string> firstSet;
    std::set<std::string> followSet;
};

class Gramatica {
private:
    std::vector<Produccion> producciones;
    std::map<int, Produccion> mapProducciones;
    std::map<std::string, NoTerminal_Atributos> first_follow_Table;
    std::unordered_map<string,unordered_map<string, int>> parsingTable;
public:
    void cargarFirstFollowTable(const std::string& pathFirst, const std::string& pathFollow, const std::string& pathNullable);
    void cargarDesdeArchivo(const std::string& nombreArchivo);
    const std::vector<Produccion>& getProducciones() const {
        return producciones;
    }
    void imprimirFirstFollowTable() const;
    std::unordered_set<std::string> noTerminales = {"Program", "StmtList", "StmtList'", "Stmt", "StmtDA", "StmtDA'", "Decl", "Assign", "IfStmt", "ForStmt", "IterSource", "PrintStmt", "ExportStmt", "FunStmt", "Type", "Expr", "CondExpr", "RelExpr", "RelExpr'", "AddExpr", "AddExpr'", "TransExpr", "TransExpr'", "ConcatExpr", "ConcatExpr'", "PrimExpr", "FuncCall", "ListLit", "IDList", "IDList'", "CompOp", "Number"};
    std::unordered_set<std::string> terminales = {"$", "ID", "if", "replay", "print", "export", "append", "remove", ":", "=", "(", ")", "then", "end", "in", "INT_LITERAL", "STRING_LITERAL", ";", ",", "Video", "Playlist", "Int", "Time", "Bool", ">", "<", "eq", "neq", "+", "-", ">>", "++", "TIME_LITERAL", "load", "duration", "speed", "WResolution", "HResolution", "Format", "clip", "[", "]"};
    std::map<std::string, std::vector<std::pair<std::string, int>>> firstWithProd;

    // Devuelve un mapa de terminales a una lista de IDs de producciones que los tienen en su FIRST
    std::map<std::string, std::vector<int>> calcularFirstProducciones() const;
    std::map<std::string, std::vector<std::pair<std::string, int>>> obtenerTablaFirstProducciones() const;

    void InitFirstWithProd();

    std::map<std::string, std::vector<std::pair<std::string, int>>> obtenerFirstsConProducciones() const;
    void buildParsingTable();

private:
    std::string trim(const std::string& s) const;
};

class Parser {
};


#endif // PARSER_H