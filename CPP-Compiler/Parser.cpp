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
    //for (auto it = root->children.begin(); it != root->children.end(); ++it) {
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
                    if (0 && child->symbol.getNombre() == selectedChild->symbol.getNombre() &&
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

// Función auxiliar para verificar si un nodo está protegido
bool isProtectedNode(TreeNode* node, const std::vector<std::string>& ast_elements) {
    return node && std::find(ast_elements.begin(), ast_elements.end(),
                           node->symbol.getNombre()) != ast_elements.end();
}

// Función recursiva para unión profunda de nodos
void deepMergeNodes(TreeNode* current, const std::vector<std::string>& ast_elements) {
    if (!current) return;

    // Primero procesar los hijos recursivamente
    for (TreeNode* child : current->children) {
        deepMergeNodes(child, ast_elements);
    }

    // Unión solo para nodos protegidos
    if (isProtectedNode(current, ast_elements)) {
        std::list<TreeNode*> mergedChildren;
        TreeNode* lastSimilar = nullptr;

        for (auto it = current->children.begin(); it != current->children.end(); ) {
            TreeNode* child = *it;

            if (lastSimilar && child->symbol.getNombre() == lastSimilar->symbol.getNombre() &&
                isProtectedNode(child, ast_elements)) {

                // Unir los hijos del nodo similar
                for (TreeNode* grandchild : child->children) {
                    lastSimilar->children.push_back(grandchild);
                    const_cast<TreeNode*>(grandchild)->parentNode = lastSimilar;
                }

                // Eliminar el nodo fusionado
                it = current->children.erase(it);
                delete child;
            } else {
                // Verificar si este hijo necesita fusionarse con sus propios hijos
                if (isProtectedNode(child, ast_elements) && !child->children.empty() &&
                    child->symbol.getNombre() == child->children.front()->symbol.getNombre() &&
                    isProtectedNode(child->children.front(), ast_elements)) {

                    // Fusionar el hijo con su primer hijo (mismo tipo)
                    TreeNode* grandchild = child->children.front();
                    child->children.pop_front();

                    // Mover todos los nietos al hijo
                    for (TreeNode* ggchild : grandchild->children) {
                        child->children.push_front(ggchild);
                        const_cast<TreeNode*>(ggchild)->parentNode = child;
                    }

                    delete grandchild;
                }

                mergedChildren.push_back(child);
                lastSimilar = child;
                ++it;
            }
        }

        current->children = mergedChildren;
    }
}

// Función iterativa wrapper para manejar todo el árbol
void mergeProtectedNodes(TreeNode* root, const std::vector<std::string>& ast_elements) {
    if (!root) return;

    // Usamos DFS iterativo para evitar posibles problemas de profundidad de recursión
    std::stack<TreeNode*> nodeStack;
    std::unordered_set<TreeNode*> processedNodes;

    nodeStack.push(root);

    while (!nodeStack.empty()) {
        TreeNode* current = nodeStack.top();

        if (processedNodes.count(current)) {
            // Si ya fue procesado, aplicar las fusiones
            deepMergeNodes(current, ast_elements);
            nodeStack.pop();
        } else {
            processedNodes.insert(current);
            // Agregar hijos en orden inverso para procesamiento DFS
            for (auto it = current->children.rbegin(); it != current->children.rend(); ++it) {
                nodeStack.push(*it);
            }
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
void mergeStmtNodesChained(TreeNode* root) {
    if (!root) return;

    std::stack<TreeNode*> nodeStack;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
        TreeNode* current = nodeStack.top();
        nodeStack.pop();

        // 1. Verificación exhaustiva del nodo actual
        if (!current || current->children.empty()) {
            continue;
        }

        // 2. Procesamiento seguro de hijos (post-order)
        for (auto it = current->children.rbegin(); it != current->children.rend(); ++it) {
            if (*it) {
                nodeStack.push(*it);
            } else {
                // Eliminar hijos nulos inmediatamente
                current->children.erase(std::next(it).base());
            }
        }

        // 3. Procesamiento específico para nodos Stmt
        if (current->symbol.getNombre() == "Stmt") {
            std::list<TreeNode*> newChildren;
            std::list<TreeNode*> stmtChildrenToMerge;

            // 4. Filtrado seguro de hijos
            for (auto it = current->children.begin(); it != current->children.end(); ) {
                TreeNode* child = *it;
                if (!child) {
                    it = current->children.erase(it);
                    continue;
                }

                if (child->symbol.getNombre() == "Stmt") {
                    stmtChildrenToMerge.push_back(child);
                    it = current->children.erase(it);
                } else {
                    // Garantizar parentNode correcto
                    const_cast<TreeNode*>(child)->parentNode = current;
                    newChildren.push_back(child);
                    ++it;
                }
            }

            // 5. Fusión segura de nodos Stmt
            for (TreeNode* stmtChild : stmtChildrenToMerge) {
                if (!stmtChild) continue;

                // Copiar profundamente el contenido del hijo Stmt
                for (TreeNode* grandchild : stmtChild->children) {
                    if (grandchild) {
                        // Crear nueva copia del nodo para evitar problemas de referencia
                        TreeNode* newChild = new TreeNode(
                            grandchild->symbol.getNombre(),
                            current,  // parentNode correcto
                            grandchild->nodeId
                        );
                        // Copiar todos los datos importantes
                        newChild->nullableValue = grandchild->nullableValue;
                        newChild->children = grandchild->children;

                        // Actualizar parentNode de los nietos
                        for (TreeNode* ggchild : newChild->children) {
                            if (ggchild) {
                                const_cast<TreeNode*>(ggchild)->parentNode = newChild;
                            }
                        }

                        newChildren.push_back(newChild);
                    }
                }

                // 6. Limpieza segura del nodo fusionado
                stmtChild->children.clear();
                delete stmtChild;
            }

            // 7. Actualización segura de la estructura
            current->children = newChildren;

            // 8. Reorganización opcional si hay muchos hijos
            if (current->children.size() > 1) {
                TreeNode* newStmtContainer = new TreeNode(
                    "Stmt",
                    current->parentNode,
                    -1  // Temporal, se asignará ID después
                );

                // Transferir hijos al nuevo contenedor
                for (TreeNode* child : current->children) {
                    if (child) {
                        newStmtContainer->children.push_back(child);
                        const_cast<TreeNode*>(child)->parentNode = newStmtContainer;
                    }
                }

                current->children.clear();
                current->children.push_back(newStmtContainer);
            }
        }
    }
}

// --- Función independiente para aplanar Stmts encadenados ---
// Modifica el árbol de parseo in-place.
void flattenChainedStmts(TreeNode* root) {
    if (!root) return;

    // Usaremos un stack para un recorrido DFS en post-orden
    // (procesar hijos antes que el nodo actual).
    std::stack<TreeNode*> s1; // Para el orden de visita (pre-orden)
    std::stack<TreeNode*> s2; // Para el orden post-orden

    s1.push(root);

    // Primera pasada: construir el orden post-orden
    while (!s1.empty()) {
        TreeNode* current = s1.top();
        s1.pop();
        s2.push(current);

        // Los hijos se empujan en orden inverso para que al sacarlos de s2
        // se procesen de izquierda a derecha (útil para ciertas operaciones, aunque no estricto aquí).
        for (auto it = current->children.rbegin(); it != current->children.rend(); ++it) {
            s1.push(*it);
        }
    }

    // Segunda pasada: procesar nodos en post-orden para aplicar el aplanamiento
    while (!s2.empty()) {
        TreeNode* current = s2.top();
        s2.pop();

        // No necesitamos `newChildrenForCurrent` a este nivel si solo modificamos
        // la lista de hijos del Stmt "cabeza" de la cadena.
        // La iteración sobre current->children es para encontrar el Stmt "cabeza".

        // Iteramos sobre los hijos de 'current'. Buscamos un 'Stmt' que sea la 'cabeza' de una cadena.
        for (auto it = current->children.begin(); it != current->children.end(); ) {
            TreeNode* child = *it;

            if (child->symbol.getNombre() == "Stmt") {
                // Verificar si este Stmt 'child' es la cabeza de una cadena:
                // Si tiene un 'Stmt' como su último hijo.
                TreeNode* lastGrandchild = nullptr;
                if (!child->children.empty()) {
                    lastGrandchild = child->children.back();
                }

                if (lastGrandchild && lastGrandchild->symbol.getNombre() == "Stmt") {
                    // ¡Patrón de cadena encontrado! `child` (Stmt cabeza) -> ... -> `lastGrandchild` (Stmt cola)
                    // Queremos que 'child' absorba el contenido de 'lastGrandchild' y los Stmt subsiguientes.

                    // Extraer todos los Stmt de la cadena y sus contenidos, y añadirlos al 'child' original.
                    TreeNode* currentStmtInChain = lastGrandchild; // Empezamos desde la cola
                    std::list<TreeNode*> accumulatedStmtContents; // Para acumular los hijos de los Stmt de la cola

                    // Primer paso: quitar el 'lastGrandchild' de los hijos de 'child'.
                    child->children.pop_back();

                    // Recorrer la cadena a partir del 'lastGrandchild' (la cola del primer Stmt)
                    while (currentStmtInChain && currentStmtInChain->symbol.getNombre() == "Stmt") {
                        TreeNode* nextStmtInChain = nullptr;

                        // Separar los hijos de contenido de 'currentStmtInChain' de su posible 'nextStmtInChain'
                        std::list<TreeNode*> currentStmtContents;
                        for (TreeNode* gc : currentStmtInChain->children) {
                            if (gc->symbol.getNombre() == "Stmt") {
                                nextStmtInChain = gc;
                            } else {
                                currentStmtContents.push_back(gc);
                            }
                        }

                        // Añadir los hijos de contenido de currentStmtInChain a la lista acumulada
                        for (TreeNode* contentChild : currentStmtContents) {
                            accumulatedStmtContents.push_back(contentChild);
                            // Actualizar parentNode, casteando de forma segura
                            const_cast<TreeNode*>(contentChild)->parentNode = child;
                        }

                        // ¡MUY IMPORTANTE! Desvincular los hijos del Stmt que vamos a borrar.
                        // Si no hacemos esto, el destructor de currentStmtInChain intentará borrar
                        // los nodos de contenido que ya hemos movido y reasignado.
                        currentStmtInChain->children.clear();

                        // Ahora podemos eliminar el nodo Stmt intermedio.
                        TreeNode* stmtToDelete = currentStmtInChain;
                        currentStmtInChain = nextStmtInChain;
                        delete stmtToDelete; // Libera la memoria del nodo Stmt intermedio
                    }

                    // Después de recolectar todos los contenidos de la cadena, añadirlos al 'child' original
                    for (TreeNode* collectedChild : accumulatedStmtContents) {
                        child->children.push_back(collectedChild);
                    }

                    // El iterador no avanza aquí porque 'child' no se ha eliminado,
                    // sino que se ha modificado in-place. La siguiente iteración
                    // del bucle 'for' para 'current->children' seguirá desde el mismo punto.
                    // Si eliminamos child, deberíamos usar it = current->children.erase(it);
                    // Pero aquí solo modificamos sus hijos, así que ++it está bien.
                    ++it;
                } else {
                    // Este Stmt 'child' no es la cabeza de una cadena (no tiene un Stmt como último hijo).
                    // Simplemente avanzamos.
                    ++it;
                }
            } else {
                // No es un Stmt, simplemente avanzamos.
                ++it;
            }
        } // Fin del bucle for sobre los hijos de 'current'
    }
}

// Función de conveniencia para ejecutar_todo el prooce
TreeNode* processAST(TreeNode* root, const std::vector<std::string>& ast_elements) {
    // Paso 1: Calcular profundidades iniciales
    calculateDepths(root);

    // Paso 2: Simplificar el AST
    //return simplifyAST(root, ast_elements);
   TreeNode* simplified =  simplifyToBinaryAST(root, ast_elements, idconter);
    //mergeProtectedNodes(simplified, ast_elements);
    flattenChainedStmts(root);
    //mergeStmtNodesChained(root);
    //reassignPositiveIds(root, idconter);
    return root;
}

TreeNode* Parser::parsing() {
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
                    currentParent->children.push_back(child);
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
    return root;
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
