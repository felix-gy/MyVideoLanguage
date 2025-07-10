#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "Scanner.h"
#include "Parser.h"
#include <sstream>
#include <vector>
using namespace std;

int main() {
    Grammar gramatica;
    gramatica.cargarDesdeArchivo("../Grammar/gramatica.txt");
    gramatica.cargarFirstFollowTable("../Grammar/list_first.txt", "../Grammar/list_follows.txt", "../Grammar/true_null.txt");
    gramatica.InitFirstWithProd();
    gramatica.buildParsingTable();
    //gramatica.imprimirFirstFollowTable();

    // FILE TEST ----------------------------------------------------------------
    //ifstream file("../Test-Files/programa.txt");
    ifstream file("../Test-Files/test.txt");
    // --------------------------------------------------------------------------
    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo.\n";
        return 1;
    }

    string source, line;
    while (getline(file, line)) source += line + '\n';

    cout << "INFO SCAN - Start scanning..\n";
    Scanner scanner(source);
    while (true) {
        Token token = scanner.gettoken();
        scanner.printToken(token);
        if (token.type == EOP) break;
    }
    cout << "INFO SCAN - Completed with " << scanner.getErrorCount() << " errors\n";

    cout << "------ INFO PARSER - Start parsing.. ------\n";
    Parser parser(gramatica, scanner);
    parser.parsing();
    cout << "------ INFO PARSER - Completed with " << parser.getErrorCount() << " errors ------\n";

    parser.printVarTable();
    return 0;

}