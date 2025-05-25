//
// Created by felixgy on 5/14/2025.
//

#include "Parser.h"
#include <iostream>



void Parser::parse() {
    // ------ PARSE TREE

    vector<ParseNodeTree> treeNodes;
    treeNodes.emplace_back(stackParser.top(), ""); // Nodo padre PROGRAM
    // ------
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
                string head =  stackParser.top();

                stackParser.pop(); // CHECK ANTES ESTABA JUNTO AL INCIALIZAR ID
                Gram.mapProducciones[id_production].print();

                vector<Symbol> production_to_stack = Gram.mapProducciones[id_production].getLadoDerecho();

                for (auto it= production_to_stack.rbegin();it !=  production_to_stack.rend(); ++it) {
                    stackParser.push(it->getNombre()); //
                }
                // PARSE TREE
                // Vincular hijos al nodo padre
                for (auto it= production_to_stack.begin();it !=  production_to_stack.end(); ++it) {
                    treeNodes.emplace_back(it->getNombre(), head);
                }
                //
                printStack(stackParser);
                if (stackParser.top() == "E") {
                    cout << "PARSER pop Epsilon" << endl;
                    stackParser.pop();
                    printStack(stackParser);
                }
            } while (input != stackParser.top());
        }
        if (flag == false && stackParser.top() == "$") {
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

    // ----- PARSE TREE
    std::ofstream file("tree_data.txt");
    for (const auto& node : treeNodes) {
        file << node.Symbol_name << "|" << node.Head_name << "\n";
    }
    file.close();
    std::cout << "Datos del arbol guardados en 'tree_data.txt'." << std::endl;
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
