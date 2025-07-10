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

TreeNode* reduceParseTreeToAST(TreeNode* node) {
    if (node == nullptr) {
        return nullptr;
    }

    std::list<TreeNode*> new_children;
    for (TreeNode* child : node->children) {
        TreeNode* processed_child = reduceParseTreeToAST(child);
        if (processed_child != nullptr) {
            new_children.push_back(processed_child);
        }
    }
    node->children = new_children;

    bool is_node_valid = false;
    for (const std::string& ast_element : AST_ELEMENTS_PRIORITY) {
        if (node->symbol.getNombre() == ast_element) {
            is_node_valid = true;
            break;
        }
    }

    if (is_node_valid) {
        return node;
    } else {
        std::vector<TreeNode*> valid_children_for_promotion;
        for (TreeNode* child : node->children) {
            if (child != nullptr) {
                bool child_is_valid_for_ast = false;
                for (const std::string& ast_element : AST_ELEMENTS_PRIORITY) {
                    if (child->symbol.getNombre() == ast_element) {
                        child_is_valid_for_ast = true;
                        break;
                    }
                }
                if (child_is_valid_for_ast) {
                    valid_children_for_promotion.push_back(child);
                }
            }
        }

        TreeNode* promoted_child = nullptr;

        if (valid_children_for_promotion.empty()) {
            delete node;
            return nullptr;
        } else if (valid_children_for_promotion.size() == 1) {
            promoted_child = valid_children_for_promotion[0];
        } else {
            auto it_chosen_child = valid_children_for_promotion.begin();
            int highest_priority_idx = -1;

            for (auto it = valid_children_for_promotion.begin(); it != valid_children_for_promotion.end(); ++it) {
                TreeNode* current_child = *it;
                auto priority_it = std::find(AST_ELEMENTS_PRIORITY.begin(), AST_ELEMENTS_PRIORITY.end(), current_child->symbol.getNombre());

                if (priority_it != AST_ELEMENTS_PRIORITY.end()) {
                    int current_priority_idx = std::distance(AST_ELEMENTS_PRIORITY.begin(), priority_it);
                    if (highest_priority_idx == -1 || current_priority_idx < highest_priority_idx) {
                        highest_priority_idx = current_priority_idx;
                        it_chosen_child = it;
                    }
                }
            }
            promoted_child = *it_chosen_child;
        }

        for (TreeNode* child : node->children) {
            if (child != promoted_child) {
                delete child;
            }
        }
        node->children.clear();

        delete node;
        return promoted_child;
    }
}

// Función para calcular y asignar las profundidades iniciales
void calculateDepths(TreeNode* node, int currentDepth = 0) {
    if (!node) return;

    node->deep = currentDepth;
    for (TreeNode* child : node->children) {
        calculateDepths(child, currentDepth + 1);
    }
}

// Función principal para simplificar el AST
TreeNode* simplifyAST(TreeNode* root, const std::vector<std::string>& ast_elements) {
    if (!root) return nullptr;

    // Procesamos los hijos recursivamente en orden inverso (derecha a izquierda)
    std::list<TreeNode*> newChildren;
    for (auto it = root->children.rbegin(); it != root->children.rend(); ++it) {
        TreeNode* simplifiedChild = simplifyAST(*it, ast_elements);
        if (simplifiedChild) {
            newChildren.push_front(simplifiedChild); // Insertar al inicio para mantener orden original
        }
    }
    root->children = newChildren;

    // Verificamos si el nodo actual debe mantenerse
    bool shouldKeep = std::find(ast_elements.begin(), ast_elements.end(), root->symbol.getNombre()) != ast_elements.end();

    if (shouldKeep || root->isVar()) {
        return root;
    } else {
        // El nodo no está en la lista, necesitamos reemplazarlo
        if (root->children.empty()) {
            delete root;
            return nullptr;
        } else if (root->children.size() == 1) {
            TreeNode* child = root->children.front();
            root->children.clear();
            delete root;
            return child;
        } else {
            // Función para evaluar la prioridad de un nodo
            auto getPriority = [&](TreeNode* node) {
                auto it = std::find(ast_elements.begin(), ast_elements.end(), node->symbol.getNombre());
                return (it != ast_elements.end()) ? std::distance(ast_elements.begin(), it) : ast_elements.size();
            };

            // Encontrar el mejor candidato según profundidad y prioridad
            TreeNode* selectedChild = nullptr;
            int minDepth = std::numeric_limits<int>::max();
            int minPriority = ast_elements.size();

            for (TreeNode* child : root->children) {
                int currentDepth = child->deep;
                int currentPriority = getPriority(child);

                if (currentDepth < minDepth ||
                   (currentDepth == minDepth && currentPriority < minPriority)) {
                    minDepth = currentDepth;
                    minPriority = currentPriority;
                    selectedChild = child;
                }
            }

            // Si no encontramos ninguno con prioridad, seleccionamos el de menor profundidad
            if (!selectedChild) {
                selectedChild = *std::min_element(
                    root->children.begin(),
                    root->children.end(),
                    [](TreeNode* a, TreeNode* b) { return a->deep < b->deep; }
                );
            }

            // Reubicar los otros hijos bajo el seleccionado
            for (TreeNode* child : root->children) {
                if (child != selectedChild) {
                    selectedChild->children.push_back(child);
                    const_cast<TreeNode*>(child)->parentNode = selectedChild;
                }
            }

            root->children.clear();
            delete root;
            return selectedChild;
        }
    }
}
int idconter = 10000;
/*
TreeNode* simplifyToBinaryAST(TreeNode* root, const std::vector<std::string>& ast_elements) {
    if (!root) return nullptr;

    // Right-to-left processing for right-recursive grammar
    std::list<TreeNode*> newChildren;
    for (auto it = root->children.rbegin(); it != root->children.rend(); ++it) {
        TreeNode* simplifiedChild = simplifyToBinaryAST(*it, ast_elements);
        if (simplifiedChild) {
            newChildren.push_front(simplifiedChild);
        }
    }
    root->children = newChildren;

    // Check if node should be kept
    bool shouldKeep = std::find(ast_elements.begin(), ast_elements.end(), root->symbol.getNombre()) != ast_elements.end();

    if (shouldKeep || root->isVar()) {
        // Ensure binary structure for kept nodes
        if (root->children.size() > 2) {
            // Convert to right-associative binary tree
            TreeNode* current = root;
            auto it = root->children.begin();
            current->children.clear();

            // First child
            if (it != root->children.end()) {
                current->children.push_back(*it);
                ++it;
            }

            // Create right branches for remaining children
            while (it != root->children.end()) {
                TreeNode* newNode = new TreeNode(root->symbol.getNombre(), current, -1); // -1 for temporary ID
                newNode->children.push_back(current->children.back());
                newNode->children.push_back(*it);
                current->children.back() = newNode;
                ++it;
            }
        }
        return root;
    } else {
        // Node needs replacement
        if (root->children.empty()) {
            delete root;
            return nullptr;
        } else if (root->children.size() == 1) {
            TreeNode* child = root->children.front();
            root->children.clear();
            delete root;
            return child;
        } else {
            // Combined priority selection (depth + list priority)
            auto getPriority = [&](TreeNode* node) {
                auto it = std::find(ast_elements.begin(), ast_elements.end(), node->symbol.getNombre());
                if (it == ast_elements.end()) return std::make_pair(INT_MAX, INT_MAX);
                return std::make_pair(node->deep, static_cast<int>(std::distance(ast_elements.begin(), it)));
            };

            TreeNode* selectedChild = nullptr;
            auto minPriority = getPriority(root->children.front());

            for (TreeNode* child : root->children) {
                auto currentPriority = getPriority(child);
                if (currentPriority < minPriority) {
                    minPriority = currentPriority;
                    selectedChild = child;
                }
            }

            if (!selectedChild) {
                selectedChild = root->children.front();
            }

            // Reorganize children under selected node
            for (TreeNode* child : root->children) {
                if (child != selectedChild) {
                    // Ensure binary structure when moving children
                    if (selectedChild->children.size() >= 2) {
                        TreeNode* newParent = new TreeNode(selectedChild->symbol.getNombre(), selectedChild, -1);
                        newParent->children = selectedChild->children;
                        selectedChild->children.clear();
                        selectedChild->children.push_back(newParent);
                    }
                    selectedChild->children.push_back(child);
                    const_cast<TreeNode*>(child)->parentNode = selectedChild;
                }
            }

            root->children.clear();
            delete root;

            // Ensure the promoted node is binary
            if (selectedChild->children.size() > 2) {
                TreeNode* newRoot = selectedChild->children.front();
                selectedChild->children.pop_front();
                for (TreeNode* child : selectedChild->children) {
                    if (newRoot->children.size() < 2) {
                        newRoot->children.push_back(child);
                    } else {
                        TreeNode* temp = new TreeNode(newRoot->symbol.getNombre(), newRoot, -1);
                        temp->children.push_back(newRoot->children.back());
                        temp->children.push_back(child);
                        newRoot->children.back() = temp;
                    }
                }
                selectedChild->children = newRoot->children;
            }

            return selectedChild;
        }
    }
}

///
TreeNode* simplifyToBinaryAST(TreeNode* root, const std::vector<std::string>& ast_elements, int& currentId) {
    if (!root) return nullptr;

    // Procesamiento de derecha a izquierda
    std::list<TreeNode*> newChildren;
    for (auto it = root->children.rbegin(); it != root->children.rend(); ++it) {
        TreeNode* simplifiedChild = simplifyToBinaryAST(*it, ast_elements, currentId);
        if (simplifiedChild) {
            newChildren.push_front(simplifiedChild);
        }
    }
    root->children = newChildren;

    bool shouldKeep = std::find(ast_elements.begin(), ast_elements.end(), root->symbol.getNombre()) != ast_elements.end();

    if (shouldKeep || root->isVar()) {
        // Asegurar estructura binaria
        if (root->children.size() > 2) {
            TreeNode* newParent = new TreeNode(root->symbol.getNombre(), root->parentNode, currentId++);
            newParent->children.push_back(root->children.front());
            root->children.pop_front();

            while (root->children.size() > 1) {
                TreeNode* rightNode = root->children.front();
                root->children.pop_front();
                newParent->children.push_back(rightNode);
                const_cast<TreeNode*>(rightNode)->parentNode = newParent;
            }

            root->children.push_front(newParent);
        }
        return root;
    } else {
        if (root->children.empty()) {
            delete root;
            return nullptr;
        } else if (root->children.size() == 1) {
            TreeNode* child = root->children.front();
            root->children.clear();
            delete root;
            return child;
        } else {
            // Selección por prioridad combinada
            auto getPriority = [&](TreeNode* node) {
                auto it = std::find(ast_elements.begin(), ast_elements.end(), node->symbol.getNombre());
                return (it != ast_elements.end()) ?
                    std::make_pair(node->deep, static_cast<int>(std::distance(ast_elements.begin(), it))) :
                    std::make_pair(INT_MAX, INT_MAX);
            };

            TreeNode* selectedChild = *std::min_element(
                root->children.begin(),
                root->children.end(),
                [&](TreeNode* a, TreeNode* b) { return getPriority(a) < getPriority(b); }
            );

            // Reorganización manteniendo estructura binaria
            for (TreeNode* child : root->children) {
                if (child != selectedChild) {
                    if (selectedChild->children.size() >= 2) {
                        TreeNode* newParent = new TreeNode(selectedChild->symbol.getNombre(), selectedChild, currentId++);
                        newParent->children = selectedChild->children;
                        selectedChild->children.clear();
                        selectedChild->children.push_back(newParent);
                    }
                    selectedChild->children.push_back(child);
                    const_cast<TreeNode*>(child)->parentNode = selectedChild;
                }
            }

            root->children.clear();
            delete root;
            return selectedChild;
        }
    }
}

//

TreeNode* simplifyToBinaryAST(TreeNode* root, const std::vector<std::string>& ast_elements, int& currentId) {
    if (!root) return nullptr;

    // Procesamiento de derecha a izquierda
    std::list<TreeNode*> newChildren;
    for (auto it = root->children.rbegin(); it != root->children.rend(); ++it) {
        TreeNode* simplifiedChild = simplifyToBinaryAST(*it, ast_elements, currentId);
        if (simplifiedChild) {
            newChildren.push_front(simplifiedChild);
        }
    }
    root->children = newChildren;

    // Caso especial para StmtDA y asignaciones
    if (root->symbol.getNombre() == "StmtDA" || root->symbol.getNombre() == "=") {
        // Asegurar que el = sea el nodo padre de su expresión
        if (root->symbol.getNombre() == "StmtDA" && root->children.size() > 1) {
            // Buscar el nodo = en los hijos
            auto it = std::find_if(root->children.begin(), root->children.end(),
                [](TreeNode* child) { return child->symbol.getNombre() == "="; });

            if (it != root->children.end()) {
                TreeNode* equalsNode = *it;
                root->children.erase(it);

                // Reorganizar la estructura
                equalsNode->children.push_back(root->children.front());
                root->children.pop_front();

                if (!root->children.empty()) {
                    equalsNode->children.push_back(root->children.front());
                    root->children.pop_front();
                }

                root->children.push_back(equalsNode);
                const_cast<TreeNode*>(equalsNode)->parentNode = root;
            }
        }
        return root;
    }

    bool shouldKeep = std::find(ast_elements.begin(), ast_elements.end(), root->symbol.getNombre()) != ast_elements.end();

    if (shouldKeep || root->isVar()) {
        // Asegurar estructura binaria
        if (root->children.size() > 2) {
            TreeNode* newParent = new TreeNode(root->symbol.getNombre(), root->parentNode, currentId++);
            newParent->children.push_back(root->children.front());
            root->children.pop_front();

            while (root->children.size() > 1) {
                TreeNode* rightNode = root->children.front();
                root->children.pop_front();
                newParent->children.push_back(rightNode);
                const_cast<TreeNode*>(rightNode)->parentNode = newParent;
            }

            root->children.push_front(newParent);
        }
        return root;
    } else {
        if (root->children.empty()) {
            delete root;
            return nullptr;
        } else if (root->children.size() == 1) {
            TreeNode* child = root->children.front();
            root->children.clear();
            delete root;
            return child;
        } else {
            // Selección por prioridad combinada
            auto getPriority = [&](TreeNode* node) {
                auto it = std::find(ast_elements.begin(), ast_elements.end(), node->symbol.getNombre());
                return (it != ast_elements.end()) ?
                    std::make_pair(node->deep, static_cast<int>(std::distance(ast_elements.begin(), it))) :
                    std::make_pair(INT_MAX, INT_MAX);
            };

            TreeNode* selectedChild = *std::min_element(
                root->children.begin(),
                root->children.end(),
                [&](TreeNode* a, TreeNode* b) { return getPriority(a) < getPriority(b); }
            );

            // Reorganización manteniendo estructura binaria
            for (TreeNode* child : root->children) {
                if (child != selectedChild) {
                    if (selectedChild->children.size() >= 2) {
                        TreeNode* newParent = new TreeNode(selectedChild->symbol.getNombre(), selectedChild, currentId++);
                        newParent->children = selectedChild->children;
                        selectedChild->children.clear();
                        selectedChild->children.push_back(newParent);
                    }
                    selectedChild->children.push_back(child);
                    const_cast<TreeNode*>(child)->parentNode = selectedChild;
                }
            }

            root->children.clear();
            delete root;
            return selectedChild;
        }
    }
}
*/

TreeNode* simplifyToBinaryAST(TreeNode* root, const std::vector<std::string>& ast_elements, int& currentId) {
    if (!root) return nullptr;

    // 1. Procesamiento recursivo (derecha a izquierda)
    std::list<TreeNode*> newChildren;
    for (auto it = root->children.rbegin(); it != root->children.rend(); ++it) {
        TreeNode* simplifiedChild = simplifyToBinaryAST(*it, ast_elements, currentId);
        if (simplifiedChild) {
            newChildren.push_front(simplifiedChild);
        }
    }
    root->children = newChildren;

    // 2. Casos especiales para operadores y StmtDA
    if (root->symbol.getNombre() == "=") {
        // Ya no forzamos estructura binaria para =
        return root;
    }
    else if (root->symbol.getNombre() == "StmtDA") {
        auto it = std::find_if(root->children.begin(), root->children.end(),
            [](TreeNode* child) { return child->symbol.getNombre() == "="; });

        if (it != root->children.end()) {
            TreeNode* equalsNode = *it;
            root->children.erase(it);

            // Agregar todos los hijos relevantes (no solo 2)
            for (auto childIt = root->children.begin(); childIt != root->children.end(); ) {
                if ((*childIt)->symbol.getNombre() == "ID" ||
                    (*childIt)->isVar() ||
                    (*childIt)->symbol.getNombre() == "Expr") {
                    equalsNode->children.push_back(*childIt);
                    childIt = root->children.erase(childIt);
                } else {
                    ++childIt;
                }
            }

            root->children.clear();
            delete root;
            return equalsNode;
        }
        return root;
    }

    // 3. Lógica principal de simplificación
    bool shouldKeep = std::find(ast_elements.begin(), ast_elements.end(), root->symbol.getNombre()) != ast_elements.end();

    if (shouldKeep || root->isVar()) {
        // Ya no forzamos estructura binaria para nodos protegidos
        return root;
    } else {
        if (root->children.empty()) {
            delete root;
            return nullptr;
        } else if (root->children.size() == 1) {
            TreeNode* child = root->children.front();
            root->children.clear();
            delete root;
            return child;
        } else {
            auto getPriority = [&](TreeNode* node) {
                auto it = std::find(ast_elements.begin(), ast_elements.end(), node->symbol.getNombre());
                return (it != ast_elements.end()) ?
                    std::make_pair(node->deep, static_cast<int>(std::distance(ast_elements.begin(), it))) :
                    std::make_pair(INT_MAX, INT_MAX);
            };

            TreeNode* selectedChild = *std::min_element(
                root->children.begin(),
                root->children.end(),
                [&](TreeNode* a, TreeNode* b) { return getPriority(a) < getPriority(b); }
            );

            // 4. Unión de nodos compatibles (versión no binaria)
            for (TreeNode* child : root->children) {
                if (child != selectedChild) {
                    // Condiciones para unión:
                    // 1. Mismo tipo de nodo
                    // 2. No es un literal/variable
                    // 3. No está en la lista de elementos protegidos
                    if (child->symbol.getNombre() == selectedChild->symbol.getNombre() &&
                        !child->isVar() &&
                        std::find(ast_elements.begin(), ast_elements.end(), child->symbol.getNombre()) == ast_elements.end()) {

                        // Unir los hijos del nodo compatible
                        for (TreeNode* grandchild : child->children) {
                            selectedChild->children.push_back(grandchild);
                            const_cast<TreeNode*>(grandchild)->parentNode = selectedChild;
                        }
                        // Eliminar el nodo fusionado
                        delete child;
                    } else {
                        // Si no se puede unir, agregar como hijo normal
                        selectedChild->children.push_back(child);
                        const_cast<TreeNode*>(child)->parentNode = selectedChild;
                    }
                }
            }

            root->children.clear();
            delete root;
            return selectedChild;
        }
    }
}

void reassignPositiveIds(TreeNode* root, int& currentId) {
    if (!root) return;

    root->nodeId = currentId++;
    for (TreeNode* child : root->children) {
        reassignPositiveIds(child, currentId);
    }
}
// Función de conveniencia para ejecutar_todo el prooce
TreeNode* processAST(TreeNode* root, const std::vector<std::string>& ast_elements) {
    // Paso 1: Calcular profundidades iniciales
    calculateDepths(root);

    // Paso 2: Simplificar el AST
    //return simplifyAST(root, ast_elements);
    return simplifyToBinaryAST(root, ast_elements, idconter);
}

void Parser::parsing() {
    // ----- PARSE TREE ------------
    int id = 0;
    treeNodesStack = std::stack<TreeNode*>();
    auto* root = new TreeNode(stackParser.top(), nullptr, id++);
    treeNodesStack.push(root); // Nodo padre PROGRAM
    // -----------------------------
    bool end_flag = false;
    bool declFlag = false;
    bool idFlag = false;
    bool error_sync_flag = false;
    string idName;
    while (true) {
        error_sync_flag = false;
        if (end_flag) {
            cout << "PARSING COMPLETADO" << endl;
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
                        errorCount++;
                        std::cout <<"[" << errorCount << "] Clave interna no encontrada." << std::endl;
                        std::cout << "Panic Mode Recovery --------------------------" << std::endl;
                        error_sync_flag = true;
                        while (!list_sync.contains(stackParser.top()))
                        {
                            treeNodesStack.pop();
                            stackParser.pop();
                        }
                        cout << "STACK MODIFICADO "<< stackParser.top() << endl;
                        printStack(stackParser);

                        Token check_token = token;
                        std::cout << " --- Skip scanner --- " << std::endl;

                        while (!(Gram.first_follow_Table[stackParser.top()].firstSet.contains(check_token.type == ID ? "ID" : check_token.value)))
                        {

                            check_token = Scan.gettoken();
                            Scan.printToken(check_token);

                            if (check_token.type == EOP)
                            {
                                end_flag = true;
                                break;
                            }
                        }
                        std::cout << " -------------------- " << std::endl;
                        //end_flag = true;
                        //break;
                        if (end_flag) break;
                        if (check_token.type == ID)
                        {
                            input = "ID";
                        }
                        else
                        {
                            input = check_token.value;
                        }
                        id_production = Gram.parsingTable[stackParser.top()][input];
                    }
                } else {
                    std::cout << "Clave externa no encontrada." << std::endl;
                    /// RESCUPERACION DE ERRORES
                    end_flag = true;
                    break;
                }
                cout << "PARSER Parsing Table [ " << stackParser.top() << " ]" << " [ " << input << " ] = ";

                if (stackParser.top() == "Decl")
                {
                    declFlag = true;
                }
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

        ///////

        if (!error_sync_flag)
        {
            if (end_flag == false && stackParser.top() == "$") {
                cout << "PARSING COMPLETADO" << endl;
                stackParser.pop();
                treeNodesStack.pop();
                break;
            }
            if (stackParser.top() == input) {

                cout << "PARSER Match: " << input <<endl;
                if (input == ";")
                {
                    declFlag = false;
                }
                if ((input == "Video"
                    || input == "Int"
                    || input == "String"
                    || input == "Playlist"
                    || input == "Bool"
                    || input == "Time") && declFlag == true && idFlag == true)
                {
                    declFlag = false;
                    idFlag = false;
                    varTable[idName] = input;
                }
                stackParser.pop();
                // ----- PARSE TREE ------------
                std::string name_sym = treeNodesStack.top()->symbol.getNombre();
                if ( name_sym == "INT_LITERAL"
                    || name_sym == "STRING_LITERAL"
                    || name_sym == "TIME_LITERAL"
                    || name_sym == "ID")
                {
                    if (name_sym == "ID")
                    {
                        idName = removeQuotes(value);
                        idFlag = true;
                    }
                    treeNodesStack.top()->nullableValue = removeQuotes(value);
                }
                treeNodesStack.pop();
                // -----------------------------
            }
            else {
                errorCount++;

                cout <<"[" << errorCount <<"] Error No concide con la Gramatica" <<endl;
                std::cout << "Panic Mode Recovery --------------------------" << std::endl;
                while (!list_sync.contains(stackParser.top()))
                {
                    treeNodesStack.pop();
                    stackParser.pop();
                }
                cout << "STACK MODIFICADO "<< stackParser.top() << endl;
                printStack(stackParser);

                Token check_token = token;
                std::cout << " --- Skip scanner --- " << std::endl;
                while (!error_sync.contains(check_token.value))
                {
                    check_token = Scan.gettoken();
                    Scan.printToken(check_token);
                    if (check_token.type == EOP)
                    {
                        end_flag = true;
                        break;
                    }
                }
                std::cout << " -------------------- " << std::endl;
                //break;
            }
        }

    }

    // ----- PARSE TREE
    exportTreeToFile(root, "ParseTree.dot");
    //exportTreeToFile(reduceParseTreeToAST(root), "ParseTree_2.dot");
    int n_counter_ids = 1;
    reassignPositiveIds(root, n_counter_ids);
    exportTreeToFile(processAST(root, AST_ELEMENTS_PRIORITY), "ParseTree_2.dot");
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

void Parser::printVarTable()
{
    cout << "Var Table de Variables" << endl;
    for (auto& v : varTable)
    {
        cout << v.first << " | " << v.second << endl;
    }
}
