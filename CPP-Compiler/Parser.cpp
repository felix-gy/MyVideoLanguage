//
// Created by felixgy on 5/14/2025.
//

#include "Parser.h"
#include <functional>
#include <iostream>

std::string removeQuotes(std::string input)
{
    while (input.size() >= 2 && input.front() == '"' && input.back() == '"') {
        input = input.substr(1, input.size() - 2);
    }
    return input;
}

void Parser::parsing() {
    // ----- PARSE TREE ------------
    int id = 0;
    treeNodesStack = std::stack<TreeNode*>();
    auto* root = new TreeNode(stackParser.top(), nullptr, id++);
    treeNodesStack.push(root); // Nodo padre PROGRAM
    // -----------------------------
    bool flag = false;
    while (true) {
        if (flag) {
            break;
        }
        Token token = Scan.gettoken();
        Scan.printToken(token);
        string value = "";
        switch (token.type) {
            case INT:
                input = "INT_LITERAL";
                value = token.value;
                break;
            case STRING:
                input = "STRING_LITERAL";
                value = token.value;
                break;
            case TIME:
                input = "TIME_LITERAL";
                value = token.value;
                break;
            case ID:
                input = "ID";
                value = token.value;
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

                vector<Symbol> production_to_stack = Gram.mapProducciones[id_production].getLadoDerecho();

                for (auto it= production_to_stack.rbegin();it !=  production_to_stack.rend(); ++it) {
                    stackParser.push(it->getNombre()); //
                }
                // ----- PARSE TREE ------------
                auto currentParent =  treeNodesStack.top();
                treeNodesStack.pop();
                // Vincular hijos al nodo padre
                for (auto it= production_to_stack.rbegin();it !=  production_to_stack.rend(); ++it) {
                    auto child = new TreeNode(it->getNombre(), currentParent, id++);
                    treeNodesStack.push(child);
                    currentParent->children.push_front(child);
                }
                // -----------------------------
                printStack(stackParser);
                if (stackParser.top() == "E") {
                    cout << "PARSER pop Epsilon" << endl;
                    stackParser.pop();
                    treeNodesStack.pop();
                    printStack(stackParser);
                }
            } while (input != stackParser.top());
        }
        if (flag == false && stackParser.top() == "$") {
            cout << "PARSING COMPLETADO CADENA ACEPTADA" << endl;
            stackParser.pop();
            treeNodesStack.pop();
            break;
        }
        if (stackParser.top() == input) {

            cout << "PARSER Match: " << input <<endl;
            stackParser.pop();
            // ----- PARSE TREE ------------
            std::string name_sym = treeNodesStack.top()->symbol.getNombre();
            if ( name_sym == "INT_LITERAL"
                || name_sym == "STRING_LITERAL"
                || name_sym == "TIME_LITERAL"
                || name_sym == "ID")
            {
                treeNodesStack.top()->nullableValue = removeQuotes(value);
            }
            treeNodesStack.pop();
            // -----------------------------
        }
        else {
            cout << "Parser Error No concide con la Gramatica" <<endl;
            break;
        }

    }

    // ----- PARSE TREE
    exportTreeToFile(root, "ParseTree.dot");
    std::cout << "Datos del arbol guardados en 'ParseTree.dot'." << std::endl;
}

void Parser::printStack(const std::stack<std::string>& _stack) {
    std::stack<std::string> tempStack = _stack;
    std::vector<std::string> vec;
    while (!tempStack.empty()) {
        vec.push_back(tempStack.top());
        tempStack.pop();
    }
    std::cout << "PARSER Stack -> "; // Imprimir el vector en orden inverso
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}
void Parser::exportTreeToFile(const TreeNode* root, const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo " << filename << std::endl;
        return;
    }
    file << "digraph Tree {\n";
    file << "    node [shape=ellipse];\n";
    function<void(const TreeNode*)> writeNode;
    writeNode = [&](const TreeNode* node) {
        if (!node) return;
        // Etiqueta del nodo
        std::string label = node->isVar() ? node->completeValue() : node->symbol.getNombre();
        file << "    node" << node->nodeId << " [label=\"" << label << "\"];\n";
        // Conexiones con hijos
        for (const TreeNode* child : node->children) {
            file << "    node" << node->nodeId << " -> node" << child->nodeId << ";\n";
            writeNode(child);
        }
    };
    writeNode(root);
    file << "}\n";
    file.close();
}
