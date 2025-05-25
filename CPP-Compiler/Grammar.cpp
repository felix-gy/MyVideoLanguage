//
// Created by felixg on 25/05/2025.
//

#include "Grammar.h"
#include <iostream>

void Production::print() const
{
    std::cout << id << " | " << ladoIzquierdo.getNombre() << " -> ";
    for (const auto& s : ladoDerecho) {
        std::cout << s.getNombre() << " ";
    }
    std::cout << std::endl;
}

void Grammar::cargarDesdeArchivo(const std::string &nombreArchivo) {

    std::ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << nombreArchivo << "\n";
        return;
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        std::stringstream ss(linea);
        std::string token;
        int id;

        std::getline(ss, token, ':');
        //std::cout << "id "<< token << std::endl;
        id = std::stoi(token);

        std::getline(ss, token, ':');
        std::string lhs = trim(token);

        std::getline(ss, token);
        std::stringstream rhs_stream(trim(token));
        std::vector<Symbol> rhs;
        while (rhs_stream >> token) {
            rhs.emplace_back(token);
        }

        producciones.emplace_back(id, Symbol(lhs), rhs);
        mapProducciones[id] = Production(id,Symbol(lhs), rhs);
    }
    archivo.close();
}



void Grammar::cargarFirstFollowTable(const std::string& pathFirst, const std::string& pathFollow, const std::string& pathNullable) {
    std::ifstream fileFirst(pathFirst);
    std::ifstream fileFollow(pathFollow);
    std::ifstream fileNullable(pathNullable);

    std::string line;

    // Leer FIRST
    while (std::getline(fileFirst, line)) {
        std::istringstream ss(line);
        std::string noTerminal;
        if (std::getline(ss, noTerminal, ':')) {
            std::string simbolo;
            std::getline(ss, simbolo);  // Lee el resto de la línea
            std::istringstream simbolosStream(simbolo);
            std::string token;

            while (std::getline(simbolosStream, token, '&')) {
                first_follow_Table[trim(noTerminal)].firstSet.insert(trim(token));
            }
        }
    }

    // Leer FOLLOW
    while (std::getline(fileFollow, line)) {
        std::istringstream ss(line);
        std::string noTerminal;
        if (std::getline(ss, noTerminal, ':')) {
            std::string simbolo;
            std::getline(ss, simbolo);
            std::istringstream simbolosStream(simbolo);
            std::string token;

            while (std::getline(simbolosStream, token, '&')) {
                first_follow_Table[trim(noTerminal)].followSet.insert(trim(token));
            }
        }
    }

    // Leer NULLABLE
    while (std::getline(fileNullable, line)) {
        std::string noTerminal = trim(line);
        std::cout << "true  "<<  noTerminal << std::endl;
        if (!noTerminal.empty()) {
            first_follow_Table[noTerminal].nullable = true;
        }
    }
}


void Grammar::imprimirFirstFollowTable() const {
    for (const auto& par : first_follow_Table) {
        const std::string& noTerminal = par.first;
        const NonTerminalFeatures& atributos = par.second;

        std::cout << "NoTerminal: " << noTerminal << "\n";
        std::cout << "  Nullable: " << (atributos.nullable ? "true" : "false") << "\n";

        std::cout << "  FIRST: { ";
        for (const auto& simbolo : atributos.firstSet) {
            std::cout << simbolo << " ";
        }
        std::cout << "}\n";

        std::cout << "  FOLLOW: { ";
        for (const auto& simbolo : atributos.followSet) {
            std::cout << simbolo << " ";
        }
        std::cout << "}\n\n";
    }
}

void Grammar::InitFirstWithProd() {
    std::map<std::string, std::vector<std::pair<std::string, int>>> resultado;

    for (const auto& prod : producciones) {
        const Symbol& ladoIzq = prod.getLadoIzquierdo();
        const std::vector<Symbol>& ladoDer = prod.getLadoDerecho();
        int idProd = prod.getId();

        // Calcular el FIRST del lado derecho
        std::set<std::string> firstsDeProduccion;

        for (const Symbol& simbolo : ladoDer) {
            if (simbolo.getNombre() == "E") {
                firstsDeProduccion.insert("E");
                break;
            }

            const auto& it = first_follow_Table.find(simbolo.getNombre());
            if (it != first_follow_Table.end()) {
                const auto& firstSet = it->second.firstSet;
                firstsDeProduccion.insert(firstSet.begin(), firstSet.end());

                if (firstSet.find("E") == firstSet.end()) {
                    break; // si no es anulable, detenemos
                }
            } else {
                if (simbolo.esTerminal()) {
                    firstsDeProduccion.insert(simbolo.getNombre());
                }
                break;
            }
        }

        // Insertar resultados
        for (const std::string& terminal : firstsDeProduccion) {
            resultado[ladoIzq.getNombre()].emplace_back(terminal, idProd);
        }
    }

    /*
     *
    // Imprimir resultado
    std::cout << "\n--- Lista de FIRSTs con sus producciones ---\n";
    for (const auto& [noTerminal, lista] : resultado) {
        std::cout << noTerminal << ":\n";
        for (const auto& [terminal, id] : lista) {
            std::cout << "  FIRST: " << terminal << " => ID: " << id << "\n";
        }
    }
    */


    // Imprimir resultado
    std::cout << "\n--- Lista de FIRSTs con sus producciones ---\n";
    for (const auto& par : resultado) { // Iteración correcta en C++11
        const std::string& noTerminal = par.first;
        const std::vector<std::pair<std::string, int>>& lista = par.second;

        std::cout << noTerminal << ":\n";
        for (const auto& elemento : lista) {
            const std::string& terminal = elemento.first;
            int id = elemento.second;
            std::cout << "  FIRST: " << terminal << " => ID: " << id << "\n";
        }
    }

    firstWithProd = resultado;
}

std::string Grammar::trim(const std::string &s) const {
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

void Grammar::buildParsingTable() {

    for (const auto& NT : noTerminales) {
        bool epsilon = false;
        int epsilonId = 0;
        for (const auto& T_id : firstWithProd[NT]) {
            if (T_id.first == "E") {
                epsilon = true;
                epsilonId = T_id.second;
                continue;
            }
            parsingTable[NT][T_id.first] = T_id.second;
        }

        if (epsilon) {
            for (const auto& f : first_follow_Table[NT].followSet ) {
                parsingTable[NT][f] = epsilonId;
            }
        }
    }

    // Imptimir tabla de parsing
    std::cout << "---------- Parsing Table LL(1) --------------" << std::endl;
    for (const auto& nt : parsingTable) {
        std::cout << nt.first << ":" << std::endl;
        for (const auto& t : nt.second ) {
            std::cout << "  " << t.first << " -> " << t.second << std::endl;
        }
    }
    std::cout << "--------------------------------------------" << std::endl;
}