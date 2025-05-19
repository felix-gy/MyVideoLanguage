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
        //std::cout << "id "<< token << std::endl;
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
    std::cout << "--------------------------------------------" << endl;
}


void Parser::parse() {
    bool flag = false;
    while (true) {
        if (flag) {
            break;
        }
        Token token = Scan.gettoken();
        Scan.printToken(token);

        switch (token.type) {
            case INT:
                input = "INT_LITERAL";
                break;
            case STRING:
                input = "STRING_LITERAL";
                break;
            case TIME:
                input = "TIME_LITERAL";
                break;
            case ID:
                input = "ID";
                break;
            default:
                input = token.value;
                break;
        }
        if (token.type == ERROR) {
            cout << "Erro en el scanner" << endl;
            break;
        }
        if (token.type == EOP) {
            cout << "Ultimo token recibido $" << endl;
        };


        int id_production;
        /// SOLO ENTRA SI ES UN NO TERMINAL EN EL STACK
        if (Gram.noTerminales.contains(stackParser.top())) {
            do {
                cout << "PARSER Input: " << input << "  StackTop: " << stackParser.top() << endl;
                /// VERIFICAR SI EXISTE EN LA TABLA DE PARSING
                if (Gram.parsingTable.find(stackParser.top()) != Gram.parsingTable.end()) {
                    // Verificar si la clave interna existe
                    if (Gram.parsingTable[stackParser.top()].find(input) != Gram.parsingTable[stackParser.top()].end()) {
                        /// ENCONTRADO
                        id_production = Gram.parsingTable[stackParser.top()][input];

                    } else {
                        /// RESCUPERACION DE ERRORES
                        std::cout << "Clave interna no encontrada." << std::endl;
                        flag = true;
                        break;
                    }
                } else {
                    std::cout << "Clave externa no encontrada." << std::endl;
                    /// RESCUPERACION DE ERRORES
                    flag = true;
                    break;
                }
                cout << "PARSER Parsing Table [ " << stackParser.top() << " ]" << " [ " << input << " ] = ";  ;
                stackParser.pop(); // CHECK ANTES ESTABA JUNTO AL INCIALIZAR ID
                Gram.mapProducciones[id_production].print();

                vector<Simbolo> production_to_stack = Gram.mapProducciones[id_production].getLadoDerecho();

                for (auto it= production_to_stack.rbegin();it !=  production_to_stack.rend(); ++it) {

                    stackParser.push(it->getNombre());
                }

                ///
                printStack(stackParser);
                if (stackParser.top() == "E") {
                    cout << "PARSER pop Epsilon" << endl;
                    stackParser.pop();
                    printStack(stackParser);
                }
            } while (input != stackParser.top());
        }
        if (stackParser.top() == "$") {
            cout << "PARSER CADENA ACEPTADA" << endl;
            stackParser.pop();
            break;
        }
        if (stackParser.top() == input) {
            cout << "PARSER Match: " << input <<endl;
            stackParser.pop();
        }
        else {
            cout << "Parser Error No concide con la Gramatica" <<endl;
            break;
        }

    }
}

void Parser::printStack(const std::stack<std::string>& _stack) {
    std::stack<std::string> tempStack = _stack; // Copia el stack
    std::vector<std::string> vec;

    // Transferir los elementos al vector
    while (!tempStack.empty()) {
        vec.push_back(tempStack.top());
        tempStack.pop();
    }

    // Imprimir el vector en orden inverso
    std::cout << "PARSER Stack -> ";
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}
