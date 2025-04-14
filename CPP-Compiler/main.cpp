#include <iostream>
#include <fstream>
#include <string>
#include "Scanner.h"

using namespace std;

int main() {
    ifstream file("programa.txt");
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
    return 0;
}