#include <iostream>
#include <fstream>
#include <Python.h>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <cstdio>
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

    wchar_t* program = Py_DecodeLocale("C:/Python310/python.exe", nullptr);
    Py_SetProgramName(program);
    // 1. Inicializar el intérprete de Python
    Py_Initialize();

    if (!Py_IsInitialized()) {
        std::cerr << "Error: No se pudo inicializar el intérprete de Python." << std::endl;
        return 1;
    }

    // Añadir el directorio actual al PYTHONPATH
    //PyRun_SimpleString("import sys\nsys.path.append('./')");
    //PyRun_SimpleString("import sys; print('Python ejecutándose desde:', sys.executable)");
    PyRun_SimpleString(R"(
import sys)");



    // Importar el módulo Python
    PyObject* pName = PyUnicode_DecodeFSDefault("output_video");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    /*
    if (pModule != nullptr) {
        // Obtener la función process_video
        PyObject* pFunc = PyObject_GetAttrString(pModule, "process_video");

        // Verificar si es callable
        if (pFunc && PyCallable_Check(pFunc)) {
            PyObject* pValue = PyObject_CallObject(pFunc, nullptr);
            Py_XDECREF(pValue);
        } else {
            PyErr_Print();
            std::cerr << "No se pudo encontrar la función 'process_video'" << std::endl;
        }

        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    } else {
        PyErr_Print();
        std::cerr << "No se pudo cargar el módulo 'video_info'" << std::endl;
    }
    */

    // Finalizar Python
    if (Py_FinalizeEx() < 0) {
        std::cerr << "Error al finalizar Python" << std::endl;
        return 1;
    }
    return 0;
}