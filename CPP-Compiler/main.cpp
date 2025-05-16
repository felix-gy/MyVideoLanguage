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

    ifstream file("../dir_files/programa.txt");
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

    Gramatica gramatica;
    gramatica.cargarDesdeArchivo("../dir_files/gramatica.txt");
    gramatica.cargarFirstFollowTable("../dir_files/list_first.txt", "../dir_files/list_follows.txt", "../dir_files/true_null.txt");


    for (const auto& prod : gramatica.getProducciones()) {
        std::cout << prod.getId() << " : " << prod.getLadoIzquierdo().getNombre() << " : ";
        for (const auto& simb : prod.getLadoDerecho()) {
            std::cout << simb.getNombre() << " ";
        }
        std::cout << "\n";
    }

    gramatica.imprimirFirstFollowTable();
    //gramatica.calcularFirstProducciones();
    //gramatica.obtenerTablaFirstProducciones();
    gramatica.InitFirstWithProd();
    gramatica.buildParsingTable();
    return 0;
}