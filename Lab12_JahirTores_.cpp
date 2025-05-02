#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

// Función para cargar el archivo conocimiento.txt en un mapa
void cargarConocimiento(map<string, string>& conocimiento, const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error al abrir el archivo de conocimiento." << endl;
        return;
    }

    string linea;
    while (getline(archivo, linea)) {
        size_t separador = linea.find('|');
        if (separador != string::npos) {
            string pregunta = linea.substr(0, separador);
            string respuesta = linea.substr(separador + 1);
            conocimiento[pregunta] = respuesta;
        }
    }

    archivo.close();
}

// Función para hacer búsqueda exacta
string buscarExacto(const map<string, string>& conocimiento, const string& pregunta) {
    auto it = conocimiento.find(pregunta);
    if (it != conocimiento.end()) {
        return it->second;
    }
    else {
        return "";
    }
}

// Función para hacer búsqueda por palabras clave
string buscarPorPalabrasClave(const map<string, string>& conocimiento, const string& pregunta) {
    vector<string> palabrasPregunta;
    stringstream ss(pregunta);
    string palabra;
    while (ss >> palabra) {
        palabrasPregunta.push_back(palabra);
    }

    for (const auto& par : conocimiento) {
        for (const auto& palabraPregunta : palabrasPregunta) {
            if (par.first.find(palabraPregunta) != string::npos) {
                return par.second;
            }
        }
    }

    return "";
}

int main() {
    map<string, string> conocimiento;
    cargarConocimiento(conocimiento, "C:\\Users\\alumno\\source\\repos\\Lab12_JahirTores_\\conocimiento.txt");

    cout << "Bienvenido al Chatbot sencillo." << endl;
    cout << "Escribe 'salir' para terminar." << endl;

    string preguntaUsuario;
    while (true) {
        cout << "\nTú: ";
        getline(cin, preguntaUsuario);

        if (preguntaUsuario == "salir") {
            cout << "Bot: ¡Hasta luego!" << endl;
            break;
        }

        // 1. Buscar coincidencia exacta
        string respuesta = buscarExacto(conocimiento, preguntaUsuario);

        // 2. Si no encuentra exacto, buscar por palabras clave
        if (respuesta.empty()) {
            respuesta = buscarPorPalabrasClave(conocimiento, preguntaUsuario);
        }

        // 3. Si no encuentra nada
        if (respuesta.empty()) {
            respuesta = "Lo siento, no conozco la respuesta a esa pregunta.";
        }

        cout << "Bot: " << respuesta << endl;
    }

    return 0;
}
