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
/*
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
*/
void PythonCodeGenerator::recorrerNodo(TreeNode* node, int indent) {
    if (!node) return;

    std::string nombre = node->symbol.getNombre();

    // 🧩 Si el nodo es 'Stmt', procesamos sus hijos como líneas independientes
    if (nombre == "Stmt") {
        for (TreeNode* hijo : node->children) {
            recorrerNodo(hijo, indent);
        }
        return;
    }

    // 🧱 Si es un bloque de control
    if (nombre == "if" || nombre == "replay_list" || nombre == "replay_range") {
        if (generadores.contains(nombre)) {
            std::string linea = generadores[nombre](node,indent);
            output << indentar(indent) << linea << "\n";
        }
        for (TreeNode* hijo : node->children) {
            recorrerNodo(hijo, indent + 1);
        }
        return;
    }

    // 🔨 Si es cualquier otro nodo con generador definido, produce línea
    if (generadores.contains(nombre)) {
        std::string linea = generadores[nombre](node, indent);
        output << indentar(indent) << linea << "\n";
    }

    // 🔁 Recorrer hijos normalmente
    for (TreeNode* hijo : node->children) {
        recorrerNodo(hijo, indent);
    }
}


void PythonCodeGenerator::generarDesdePrograma(TreeNode* root) {
    if (!root || root->symbol.getNombre() != "Program") {
        std::cerr << "Error: El nodo raíz no es 'programa'.\n";
        return;
    }

    // Cabecera de MoviePy



    output << "# Código del Compilador C++\n";
    output << "from moviepy import *\n";
    output << "from moviepy.video.fx import *\n";
    output << "import math\n";
    output << "import os \n\n";


    // Buscar el nodo StatementList
    TreeNode* statements= *root->children.begin();
    string statements_name = statements->symbol.getNombre();
    output << generadores[statements_name](statements, 0) << "\n";
}


std::string formatTimeToMoviePyTupleString(const std::string& time_str) {
    // Validar formato básico (HH:MM:SS)
    if (time_str.length() != 8 || time_str[2] != ':' || time_str[5] != ':') {
        // Podrías lanzar una excepción o devolver un string de error.
        // Para este ejemplo, devolvemos un string vacío o un error.
        return "// ERROR: Formato de tiempo inválido. Se espera HH:MM:SS";
    }

    int hours = 0;
    int minutes = 0;
    int seconds = 0;

    try {
        // Extraer horas, minutos y segundos
        hours = std::stoi(time_str.substr(0, 2));
        minutes = std::stoi(time_str.substr(3, 2));
        seconds = std::stoi(time_str.substr(6, 2));
    } catch (const std::exception& e) {
        return "// ERROR: Valores de tiempo no numéricos. " + std::string(e.what());
    }

    // Validar rangos (opcional pero recomendado)
    if (hours < 0 || hours > 99 || // Horas pueden ser más de 24 en duración total
        minutes < 0 || minutes >= 60 ||
        seconds < 0 || seconds >= 60) {
        return "// ERROR: Valores de tiempo fuera de rango (MM/SS > 59).";
        }

    std::ostringstream oss;

    // MoviePy acepta (M, S) o (H, M, S).
    // Si las horas son 0, es más conciso usar (M, S).
    // De lo contrario, usar (H, M, S).
    if (hours == 0) {
        oss << "(" << minutes << ", " << seconds << ")";
    } else {
        oss << "(" << hours << ", " << minutes << ", " << seconds << ")";
    }

    return oss.str();
}

void PythonCodeGenerator::inicializarGeneradores(){
    this->generadores.clear();
    this->generadores = {
        {"Stmt", [this](TreeNode* node, int ident) {
            std::ostringstream res;
            for (TreeNode* hijo : node->children)
            {
                string nombre = hijo->symbol.getNombre();
                std::string linea = this->generadores[nombre](hijo, ident);
                res << this->indentar(ident) << linea << "\n";
            }
            return res.str();
        }},
        {"load", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string filePath = (*it)->nullableValue;
            return "VideoFileClip(\"" + filePath + "\")";
        }},
        {"clip", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string t2 = (*it)->nullableValue; ++it;
            std::string t1 = (*it)->nullableValue; ++it;
            std::string source = (*it)->nullableValue;
            return  source + ".subclipped(" + t1 + ", " + t2 + ")";
        }},
        {"speed", [this](TreeNode* node, int ident) {
            std::ostringstream speed_res;
            auto it = node->children.begin();
            std::string factor = (*it)->nullableValue; ++it;
            std::string video = (*it)->nullableValue;
            speed_res << video +".with_effects([vfx.MultiplySpeed(" + factor + ")]) \n";
            return speed_res.str();
            //speed_res << video +".time_transform(lambda t: t * " + factor + ") \n";
            //speed_res << video + " = " + video +".set_duration("+""
            //videoFinal = videoFinal.set_duration(videoConcatenado.duration * 2)
        }},
        {"++", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string left = (*it)->nullableValue; ++it;
            std::string right = (*it)->nullableValue;
            return "concatenate_videoclips([" + right + ", " + left + "])";
        }},
        {">>", [this](TreeNode* node, int ident) {

            auto it = node->children.begin();
            std::string b_var_name = (*it)->nullableValue; ++it;  // Segundo clip (entra)
            std::string a_var_name = (*it)->nullableValue;        // Primer clip (sale)

            std::string transition_duration = "3"; // Ajustable: duración en segundos de la transición

            std::ostringstream python_expression;
            python_expression << "CompositeVideoClip([\n";

            // Primer clip: se desliza hacia la izquierda al final
            python_expression << this->indentar(ident + 1)
                              << a_var_name << ".with_effects([vfx.SlideOut("
                              << transition_duration << ", side='left')]),\n";

            // Segundo clip: entra desde la derecha justo antes del final del primero
            python_expression << this->indentar(ident + 1)
                              << b_var_name << ".with_start("
                              << a_var_name << ".duration - "
                              << transition_duration << ").with_effects([vfx.SlideIn("
                              << transition_duration << ", side='right')]),\n";

            python_expression << this->indentar(ident) << "])";

            return python_expression.str();
        }},
        {"export", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string string_out = (*it)->nullableValue; ++it;
            std::string id_name = (*it)->nullableValue;
            return id_name + ".write_videofile(\"" + string_out + "\")";
            //return id_name + ".write_videofile(\"" + string_out + "\", codec='libx264', audio_codec='aac')";

        }},
        {"duration", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string video = (*it)->nullableValue;
            return "("+ video +".duration)";
        }},
        {"WResolution", [this](TreeNode* node, int ident) {
            std::ostringstream res_WResolution;
            auto it = node->children.begin();
            std::string video = (*it)->nullableValue;
            //res_WResolution << "print(f\"Ancho ( "<<video<< " ) : { " << video << ".w }\")";
            res_WResolution << "print(f\"Ancho ( "<<video<< " ) : { " << video << ".w }\")";
            return res_WResolution.str();
        }},
        {"HResolution", [this](TreeNode* node, int ident) {
            std::ostringstream res_HResolution;
            auto it = node->children.begin();
            std::string video = (*it)->nullableValue;
            //res_HResolution << "print(f\"Ancho ( "<<video<< " ) : { " << video << ".h }\")";
            return res_HResolution.str();
        }},
        {"Format", [this](TreeNode* node, int ident) {
            /*
            std::ostringstream res_resplay;
            res_resplay << "import os \n";
            auto it = node->children.begin();
            std::string output = (*it)->completeValue(); ++it;
            std::string video = (*it)->completeValue();
            return output + " = " + video + ".filename.split('.')[-1]";
            */
            return "";
        }},
        {"print", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string msg = (*it)->nullableValue;
            return "print(\"" + msg + "\")";
        }},
        {"=", [this](TreeNode* node, int ident) {
            auto operador_asign = node->children.begin();
            string operador;
            if ((*operador_asign)->isVar())
            {
                if ((*operador_asign)->symbol.getNombre() == "TIME_LITERAL")
                {
                    operador = formatTimeToMoviePyTupleString((*operador_asign)->nullableValue);
                }
                else
                {
                    operador = (*operador_asign)->nullableValue;
                }

            }
            else
            {
                operador = this->generadores[(*operador_asign)->symbol.getNombre() ](*operador_asign, ident);
            }
            auto id_res = next(operador_asign);
            string id_res_name = (*id_res)->nullableValue;
            return id_res_name + " = " + operador;
        }},
        {"eq", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string left;
            if ((*it)->isVar())
            {
                left = (*it)->nullableValue;
            }
            else
            {
                left = generadores[(*it)->symbol.getNombre() ](*it, ident);
            }
            ++it;
            std::string right;
            if ((*it)->isVar())
            {
                right = (*it)->nullableValue;
            }
            else
            {
                right = generadores[(*it)->symbol.getNombre() ](*it, ident);
            }
            //std::string right = (*it)->nullableValue;
            return right + " == " + left;
        }},
        {"neq", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string left;
            if ((*it)->isVar())
            {
                left = (*it)->nullableValue;
            }
            else
            {
                left = generadores[(*it)->symbol.getNombre() ](*it, ident);
            }
            ++it;
            std::string right;
            if ((*it)->isVar())
            {
                right = (*it)->nullableValue;
            }
            else
            {
                right = generadores[(*it)->symbol.getNombre() ](*it, ident);
            }
            return right + " != " + left;
        }},
        {">", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string left;
            if ((*it)->isVar())
            {
                left = (*it)->nullableValue;
            }
            else
            {
                left = generadores[(*it)->symbol.getNombre() ](*it, ident);
            }
            ++it;
            std::string right;
            if ((*it)->isVar())
            {
                right = (*it)->nullableValue;
            }
            else
            {
                right = generadores[(*it)->symbol.getNombre() ](*it, ident);
            }
            return right + " > " + left;
        }},
        {"<", [this](TreeNode* node, int ident) {
            auto it = node->children.begin();
            std::string left;
            if ((*it)->isVar())
            {
                left = (*it)->nullableValue;
            }
            else
            {
                left = generadores[(*it)->symbol.getNombre() ](*it, ident);
            }
            ++it;
            std::string right;
            if ((*it)->isVar())
            {
                right = (*it)->nullableValue;
            }
            else
            {
                right = generadores[(*it)->symbol.getNombre() ](*it, ident);
            }
            return right + " > " + left;
        }},
        {"+", [this](TreeNode* node, int ident) {
            std::string res;
            for (auto it = node->children.begin(); it != node->children.end(); ++it)
            {
                if (std::next(it) == node->children.end())
                {
                    if ((*it)->isVar())
                    {
                        res +=  (*it)->nullableValue;
                    }
                    else
                    {
                        res+= generadores[(*it)->symbol.getNombre() ](*it, ident);
                    }
                }
                else
                {
                    if ((*it)->isVar())
                    {
                        res +=  (*it)->nullableValue +  " + ";
                    }
                    else
                    {
                        res+= generadores[(*it)->symbol.getNombre() ](*it, ident) +  " + ";
                    }
                }
            }
            return res;
        }},
        {"-", [this](TreeNode* node, int ident) {
            std::string res;
            for (auto it = node->children.begin(); it != node->children.end(); ++it)
            {
                if (std::next(it) == node->children.end())
                {
                    if ((*it)->isVar())
                    {
                        res +=  (*it)->nullableValue;
                    }
                    else
                    {
                        res+= generadores[(*it)->symbol.getNombre() ](*it, ident);
                    }
                }
                else
                {
                    if ((*it)->isVar())
                    {
                        res +=  (*it)->nullableValue +  " - ";
                    }
                    else
                    {
                        res+= generadores[(*it)->symbol.getNombre() ](*it, ident) +  " - ";
                    }
                }
            }
            return res;
        }},
        {"Append", [this](TreeNode* node, int ident) {
            /*
            auto it = node->children.begin();
            std::string video = (*it)->completeValue(); ++it;
            std::string playlist = (*it)->completeValue();
            return playlist + ".append(" + video + ")";
            */
            return "";
        }},
        {"Remove", [this](TreeNode* node, int ident) {
            /*
            auto it = node->children.begin();
            std::string video = (*it)->completeValue(); ++it;
            std::string playlist = (*it)->completeValue();
            return playlist + ".remove(" + video + ")";
            */
            return "";
        }},
        {"replay_list", [this](TreeNode* node, int ident) {
            /*
            auto it = node->children.begin();
            std::string var = (*it)->completeValue(); ++it;
            std::string collection = (*it)->completeValue();
            return "for " + var + " in " + collection + ":";
            */
            return "";
        }},
        {"replay", [this](TreeNode* node, int ident) {

            std::ostringstream res_resplay;
            auto first_stmt = node->children.begin();
            TreeNode* first_hijo = *node->children.begin();
            auto second_range_i = std::next(first_stmt);
            auto third_in = std::next(second_range_i);
            auto four_var_literal = std::next(third_in);

            string stmt = this->generadores[first_hijo->symbol.getNombre()](first_hijo, ident+1);
            res_resplay << "for " + (*four_var_literal)->nullableValue + " in range(" + (*second_range_i)->nullableValue + "): \n" << stmt;
            return res_resplay.str();
        }},
        {"if", [this](TreeNode* node, int ident) {
            std::ostringstream res_if;
            auto first_it_stmt = node->children.begin();
            auto secont_it_condition = next(first_it_stmt);
            string stmt = this->generadores[(*first_it_stmt)->symbol.getNombre()](*first_it_stmt, ident+1);
            string condition = this->generadores[(*secont_it_condition)->symbol.getNombre()](*secont_it_condition, ident+1);

            res_if << "if " + condition + ":\n" << stmt;
            return res_if.str();
        }},
    };
}