//
// Created by felixg on 10/07/2025.
//

#ifndef PYTHONCODEGENERATOR_H
#define PYTHONCODEGENERATOR_H



#include <string>
#include <list>
#include <unordered_map>
#include <functional>
#include <sstream>
#include "Parser.h"

class PythonCodeGenerator {
private:
    std::ostringstream output;
    //std::unordered_map<std::string, std::function<std::string(TreeNode)>> generadores;
    std::unordered_map<std::string, std::function<std::string(TreeNode*, int)>> generadores;

    void inicializarGeneradores();
    void recorrerNodo(TreeNode* node, int indent = 0);
    std::string indentar(int nivel);

public:
    PythonCodeGenerator();

    void generarDesdePrograma(TreeNode* root);
    void generarDesdeAST(TreeNode* root);
    void guardarEnArchivo(const std::string& path);
};


#endif //PYTHONCODEGENERATOR_H
