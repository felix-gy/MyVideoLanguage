//
// Created by felixgy on 5/14/2025.
//

#include "Parser.h"

void Gramatica::cargarDesdeArchivo(const std::string &nombreArchivo) {

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
        std::cout << "id "<< token << std::endl;
        id = std::stoi(token);

        std::getline(ss, token, ':');
        std::string lhs = trim(token);

        std::getline(ss, token);
        std::stringstream rhs_stream(trim(token));
        std::vector<Simbolo> rhs;
        while (rhs_stream >> token) {
            rhs.emplace_back(token);
        }

        producciones.emplace_back(id, Simbolo(lhs), rhs);
        mapProducciones[id] = Produccion(id,Simbolo(lhs), rhs);
    }
    archivo.close();
}



void Gramatica::cargarFirstFollowTable(const std::string& pathFirst, const std::string& pathFollow, const std::string& pathNullable) {
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


void Gramatica::imprimirFirstFollowTable() const {
    for (const auto& par : first_follow_Table) {
        const std::string& noTerminal = par.first;
        const NoTerminal_Atributos& atributos = par.second;

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

std::map<std::string, std::vector<int>> Gramatica::calcularFirstProducciones() const {
    std::map<std::string, std::vector<int>> resultado;

    for (const Produccion& prod : producciones) {
        const Simbolo& lhs = prod.getLadoIzquierdo();
        const std::vector<Simbolo>& rhs = prod.getLadoDerecho();

        bool contieneEpsilon = false;
        std::set<std::string> firstSet;

        for (const Simbolo& simbolo : rhs) {
            std::string nombre = simbolo.getNombre();

            if (nombre == "E") {
                contieneEpsilon = true;
                break;
            }

            if (simbolo.esTerminal()) {
                firstSet.insert(nombre);
                break;
            }

            // Verificar si el no terminal está en la tabla
            auto it = first_follow_Table.find(nombre);
            if (it != first_follow_Table.end()) {
                const auto& atributo = it->second;

                for (const std::string& terminal : atributo.firstSet) {
                    if (terminal != "E") {
                        firstSet.insert(terminal);
                    }
                }

                // Si contiene epsilon, seguimos con el siguiente símbolo
                if (atributo.firstSet.find("E") != atributo.firstSet.end()) {
                    continue;
                } else {
                    break;
                }
            } else {
                std::cerr << "Advertencia: No se encontró el no terminal '" << nombre << "' en la tabla FIRST.\n";
                break;
            }
        }

        // Agregar terminales encontrados al resultado
        for (const std::string& terminal : firstSet) {
            resultado[terminal].push_back(prod.getId());
        }

        // Manejar producciones con ε explícito (solo "E" como lado derecho)
        if (rhs.size() == 1 && rhs[0].getNombre() == "E") {
            resultado["E"].push_back(prod.getId());
        }
    }

    // Imprimir el resultado
    std::cout << "First-Terminal -> Lista de IDs de Producciones:\n";
    for (const auto& [terminal, lista] : resultado) {
        std::cout << terminal << " -> { ";
        for (int id : lista) {
            std::cout << id << " ";
        }
        std::cout << "}\n";
    }

    return resultado;
}
std::map<std::string, std::vector<std::pair<std::string, int>>> Gramatica::obtenerTablaFirstProducciones() const {
    std::map<std::string, std::vector<std::pair<std::string, int>>> resultado;

    for (const Produccion& produccion : producciones) {
        const std::string& noTerminal = produccion.getLadoIzquierdo().getNombre();
        const std::vector<Simbolo>& ladoDerecho = produccion.getLadoDerecho();
        int prodID = produccion.getId();

        std::set<std::string> firstDeProduccion;

        // Caso especial: Producción → E (epsilon)
        if (ladoDerecho.size() == 1 && ladoDerecho[0].getNombre() == "E") {
            firstDeProduccion.insert("ε");
        } else {
            bool todosAnulables = true;

            for (const Simbolo& simbolo : ladoDerecho) {
                const std::string& nombreSimbolo = simbolo.getNombre();

                if (simbolo.esTerminal()) {
                    firstDeProduccion.insert(nombreSimbolo);
                    todosAnulables = false;
                    break;
                }

                auto it = first_follow_Table.find(nombreSimbolo);
                if (it != first_follow_Table.end()) {
                    const NoTerminal_Atributos& atributos = it->second;
                    for (const std::string& f : atributos.firstSet) {
                        if (f != "ε") {
                            firstDeProduccion.insert(f);
                        }
                    }

                    if (!atributos.nullable) {
                        todosAnulables = false;
                        break;
                    }
                } else {
                    std::cerr << "[Warning] No se encontró el símbolo no terminal '" << nombreSimbolo << "' en la tabla FIRST/FOLLOW.\n";
                    todosAnulables = false;
                    break;
                }
            }

            if (todosAnulables) {
                firstDeProduccion.insert("ε");
            }
        }

        // Insertamos cada FIRST encontrado junto con la producción
        for (const std::string& first : firstDeProduccion) {
            resultado[first].emplace_back(noTerminal, prodID);
        }
    }

    // Impresión para verificación
    std::cout << "\n=== Tabla de FIRST por Producción ===\n";
    for (const auto& [first, listaProducciones] : resultado) {
        std::cout << "FIRST: " << first << "\n";
        for (const auto& [noTerminal, idProd] : listaProducciones) {
            std::cout << "  - " << noTerminal << " → Producción ID: " << idProd << "\n";
        }
    }
    std::cout << "=== Fin de tabla ===\n";

    return resultado;
}

std::map<std::string, std::vector<std::pair<std::string, int>>> Gramatica::obtenerFirstsConProducciones() const {
    std::map<std::string, std::vector<std::pair<std::string, int>>> resultado;

    for (const auto& prod : producciones) {
        const Simbolo& ladoIzq = prod.getLadoIzquierdo();
        const std::vector<Simbolo>& ladoDer = prod.getLadoDerecho();
        int idProd = prod.getId();

        // Calcular el FIRST del lado derecho
        std::set<std::string> firstsDeProduccion;

        for (const Simbolo& simbolo : ladoDer) {
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

    // Imprimir resultado
    std::cout << "\n--- Lista de FIRSTs con sus producciones ---\n";
    for (const auto& [noTerminal, lista] : resultado) {
        std::cout << noTerminal << ":\n";
        for (const auto& [terminal, id] : lista) {
            std::cout << "  FIRST: " << terminal << " => Producción ID: " << id << "\n";
        }
    }

    return resultado;
}


void Gramatica::InitFirstWithProd() {
    std::map<std::string, std::vector<std::pair<std::string, int>>> resultado;

    for (const auto& prod : producciones) {
        const Simbolo& ladoIzq = prod.getLadoIzquierdo();
        const std::vector<Simbolo>& ladoDer = prod.getLadoDerecho();
        int idProd = prod.getId();

        // Calcular el FIRST del lado derecho
        std::set<std::string> firstsDeProduccion;

        for (const Simbolo& simbolo : ladoDer) {
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

std::string Gramatica::trim(const std::string &s) const {
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

void Gramatica::buildParsingTable() {

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
    std::cout << "---------- Parsing Table LL(1) --------------" << endl;
    for (const auto& nt : parsingTable) {
        cout << nt.first << ":" << endl;
        for (const auto& t : nt.second ) {
            cout << "  " << t.first << " -> " << t.second << endl;
        }
    }


}
