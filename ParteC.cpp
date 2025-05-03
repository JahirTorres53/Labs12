#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <curl.h>

using namespace std;

const string OPENAI_API_KEY = "";//no me dejo subirlo con la api

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

string buscarExacto(const map<string, string>& conocimiento, const string& pregunta) {
    auto it = conocimiento.find(pregunta);
    if (it != conocimiento.end()) {
        return it->second;
    }
    return "";
}

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

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

string extraerContenido(const string& respuestaRaw) {
    size_t posInicio = respuestaRaw.find("\"content\":\"");
    if (posInicio == string::npos) return "No se pudo obtener respuesta.";

    posInicio += 11; // Saltar el texto `"content":"`
    size_t posFin = respuestaRaw.find("\"", posInicio);
    if (posFin == string::npos) return "No se pudo obtener respuesta.";

    string contenido = respuestaRaw.substr(posInicio, posFin - posInicio);

    // Reemplazar caracteres de escape básicos
    size_t pos = 0;
    while ((pos = contenido.find("\\n", pos)) != string::npos) {
        contenido.replace(pos, 2, "\n");
    }
    while ((pos = contenido.find("\\\"", pos)) != string::npos) {
        contenido.replace(pos, 2, "\"");
    }

    return contenido;
}

string consultarOpenAI(const string& pregunta) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error al inicializar CURL.";

    string respuesta;
    string cuerpoJSON =
        "{"
        "\"model\": \"gpt-3.5-turbo\","
        "\"messages\": ["
        "{\"role\": \"user\", \"content\": \"" + pregunta + "\"}"
        "]"
        "}";

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + OPENAI_API_KEY).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cuerpoJSON.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        return "Error al consultar OpenAI.";
    }

    return extraerContenido(respuesta);
}

int main() {
    map<string, string> conocimiento;
    cargarConocimiento(conocimiento, "conocimiento.txt");

    cout << "Bienvenido al Chatbot con OpenAI (sin JSON externo)." << endl;
    cout << "Escribe 'salir' para terminar." << endl;

    string preguntaUsuario;
    while (true) {
        cout << "\nTú: ";
        getline(cin, preguntaUsuario);

        if (preguntaUsuario == "salir") {
            cout << "Bot: ¡Hasta luego!" << endl;
            break;
        }

        string respuesta = buscarExacto(conocimiento, preguntaUsuario);
        if (respuesta.empty()) {
            respuesta = buscarPorPalabrasClave(conocimiento, preguntaUsuario);
        }
        if (respuesta.empty()) {
            respuesta = consultarOpenAI(preguntaUsuario);
        }

        cout << "Bot: " << respuesta << endl;
    }

    return 0;
}
