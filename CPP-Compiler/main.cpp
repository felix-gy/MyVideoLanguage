#include <iostream>
#include <fstream>
#include <Python.h>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include "Scanner.h"
#include "Parser.h"
#include "PythonCodeGenerator.h"

using namespace std;

int main() {
    // --- Inicializar Gramática ---
    Grammar gramatica;
    gramatica.cargarDesdeArchivo("../Grammar/gramatica.txt");
    gramatica.cargarFirstFollowTable("../Grammar/list_first.txt", "../Grammar/list_follows.txt", "../Grammar/true_null.txt");
    gramatica.InitFirstWithProd();
    gramatica.buildParsingTable();

    // --- Leer archivo fuente ---
    ifstream file("../Test-Files/test.txt");
    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo.\n";
        return 1;
    }

    string source, line;
    while (getline(file, line))
        source += line + '\n';

    // --- Escaneo ---
    cout << "INFO SCAN - Start scanning..\n";
    Scanner scanner(source);
    while (true) {
        Token token = scanner.gettoken();
        scanner.printToken(token);
        if (token.type == EOP) break;
    }
    cout << "INFO SCAN - Completed with " << scanner.getErrorCount() << " errors\n";

    // --- Parsing ---
    cout << "------ INFO PARSER - Start parsing.. ------\n";
    Parser parser(gramatica, scanner);
    TreeNode* ast = parser.parsing();  // Retorna el AST
    cout << "------ INFO PARSER - Completed with " << parser.getErrorCount() << " errors ------\n";
    parser.printVarTable();

    // --- Generar Código Python ---
    PythonCodeGenerator generator;
    generator.generarDesdePrograma(ast);
    generator.guardarEnArchivo("output_video.py");

    cout << "Código Python generado en 'output_video.py'\n";
    return 0;
}