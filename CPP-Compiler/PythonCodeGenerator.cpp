#include "PythonCodeGenerator.h"
#include <fstream>
#include <limits>
#include <algorithm>
#include <iostream>

PythonCodeGenerator::PythonCodeGenerator() {
    inicializarGeneradores();
}

std::string PythonCodeGenerator::indentar(int nivel) {
    return std::string(nivel * 4, ' ');
}

void PythonCodeGenerator::generarDesdeAST(TreeNode* root) {
    output << "from moviepy.editor import *\n";
    output << "from moviepy.video.fx import *\n\n";
    recorrerNodo(root);
}

void PythonCodeGenerator::guardarEnArchivo(const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "Error al escribir el archivo Python: " << path << std::endl;
        return;
    }
    out << output.str();
    out.close();
}

void PythonCodeGenerator::recorrerNodo(TreeNode* node, int indent) {
    if (!node) return;

    std::string nombre = node->symbol.getNombre();

    // Generador directo
    if (generadores.contains(nombre)) {
        std::string linea = generadores[nombre](node);
        output << indentar(indent) << linea << "\n";
    }

    // Bloques condicionales o iterativos
    if (nombre == "if" || nombre == "replay_list" || nombre == "replay_range") {
        output << indentar(indent) << generadores[nombre](node) << "\n";
        for (TreeNode* hijo : node->children) {
            recorrerNodo(hijo, indent + 1);
        }
    } else {
        for (TreeNode* hijo : node->children) {
            recorrerNodo(hijo, indent);
        }
    }
}

void PythonCodeGenerator::inicializarGeneradores() {
    generadores = {
        {"load", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string videoId = (*it)->completeValue(); ++it;
            std::string filePath = (*it)->completeValue();
            return videoId + " = VideoFileClip(\"" + filePath + "\")";
        }},
        {"clip", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string output = (*it)->completeValue(); ++it;
            std::string source = (*it)->completeValue(); ++it;
            std::string t1 = (*it)->completeValue(); ++it;
            std::string t2 = (*it)->completeValue();
            return output + " = " + source + ".subclip(" + t1 + ", " + t2 + ")";
        }},
        {"speed", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string video = (*it)->completeValue(); ++it;
            std::string factor = (*it)->completeValue();
            return video + " = " + video + ".fx(vfx.speedx, " + factor + ")";
        }},
        {"++", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string output = (*it)->completeValue(); ++it;
            std::string left = (*it)->completeValue(); ++it;
            std::string right = (*it)->completeValue();
            return output + " = concatenate_videoclips([" + left + ", " + right + "])";
        }},
        {">>", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string result = (*it)->completeValue(); ++it;
            std::string a = (*it)->completeValue(); ++it;
            std::string b = (*it)->completeValue();
            return result + " = concatenate_videoclips([" + a + ".crossfadeout(1), " + b + ".crossfadein(1)])";
        }},
        {"export", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string source = (*it)->completeValue(); ++it;
            std::string name = (*it)->completeValue();
            return source + ".write_videofile(\"" + name + ".mp4\")";
        }},
        {"duration", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string var = (*it)->completeValue(); ++it;
            std::string video = (*it)->completeValue();
            return var + " = " + video + ".duration";
        }},
        {"WResolution", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string output = (*it)->completeValue(); ++it;
            std::string video = (*it)->completeValue();
            return output + " = " + video + ".w";
        }},
        {"HResolution", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string output = (*it)->completeValue(); ++it;
            std::string video = (*it)->completeValue();
            return output + " = " + video + ".h";
        }},
        {"Format", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string output = (*it)->completeValue(); ++it;
            std::string video = (*it)->completeValue();
            return output + " = " + video + ".filename.split('.')[-1]";
        }},
        {"print", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string msg = (*it)->completeValue();
            return "print(" + msg + ")";
        }},
        {"=", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string left = (*it)->completeValue(); ++it;
            std::string right = (*it)->completeValue();
            return left + " = " + right;
        }},
        {"eq", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string left = (*it)->completeValue(); ++it;
            std::string right = (*it)->completeValue();
            return left + " == " + right;
        }},
        {"neq", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string left = (*it)->completeValue(); ++it;
            std::string right = (*it)->completeValue();
            return left + " != " + right;
        }},
        {">", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string left = (*it)->completeValue(); ++it;
            std::string right = (*it)->completeValue();
            return left + " > " + right;
        }},
        {"<", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string left = (*it)->completeValue(); ++it;
            std::string right = (*it)->completeValue();
            return left + " < " + right;
        }},
        {"+", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string left = (*it)->completeValue(); ++it;
            std::string right = (*it)->completeValue();
            return left + " + " + right;
        }},
        {"-", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string left = (*it)->completeValue(); ++it;
            std::string right = (*it)->completeValue();
            return left + " - " + right;
        }},
        {"Append", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string video = (*it)->completeValue(); ++it;
            std::string playlist = (*it)->completeValue();
            return playlist + ".append(" + video + ")";
        }},
        {"Remove", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string video = (*it)->completeValue(); ++it;
            std::string playlist = (*it)->completeValue();
            return playlist + ".remove(" + video + ")";
        }},
        {"replay_list", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string var = (*it)->completeValue(); ++it;
            std::string collection = (*it)->completeValue();
            return "for " + var + " in " + collection + ":";
        }},
        {"replay_range", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string var = (*it)->completeValue(); ++it;
            std::string count = (*it)->completeValue();
            return "for " + var + " in range(" + count + "):";
        }},
        {"if", [](TreeNode* node) {
            auto it = node->children.begin();
            std::string condition = (*it)->completeValue();
            return "if " + condition + ":";
        }},
    };
}