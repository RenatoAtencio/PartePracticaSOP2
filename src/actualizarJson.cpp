#include <iostream>
#include <fstream>
#include "../include/json.hpp"

using namespace std;
using json = nlohmann::json;

// Función para cargar datos desde un archivo JSON
json loadJSON(const string& filename) {
    json jsonData;
    ifstream file(filename);
    if (file.is_open()) {
        file >> jsonData;
        file.close();
    }
    return jsonData;
}

// Función para guardar datos en un archivo JSON
void saveJSON(const string& filename, const json& data, int memory) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << data.dump(memory);  
        outFile.close();
    }
    else {
        cerr << "No se pudo abrir el archivo para escritura." << endl;
    }
}

// Función para agregar un nuevo objeto JSON y mantener solo los últimos 4 objetos
void addAndMaintain(json& data, const json& newObject, int memory) {
    data.push_back(newObject);
    if (data.size() > static_cast<size_t>(memory)) {
        data.erase(data.begin());
    }
}

int main(int argc, char* argv[]) {
    /* la entrada por consola debe ser asi:
   ./programa '{"origen":"./invertedindex","destino":"./memcache","contexto":{"tiempo":"1000ns","ori":"backend","isFound":true,"busqueda":"",
   "resultados":[{"archivo":"file-text1.txt","puntaje":"122"},{"archivo":"file-text20.txt","puntaje":"34"},{"archivo":"file-text3.txt","puntaje":"10"},{"archivo":"file-text2.txt","puntaje":"2"}]}}'
    4*/
    // Verificar si se proporciona un mensaje como argumento de línea de comandos
    if (argc != 3) {
        cerr << "Se debe ingresar: " << argv[0] << " mensaje memory" << endl;
        return 1;
    }

    // Obtener el mensaje desde la línea de comandos
    string mensaje = argv[1];
    int memory = atoi(argv[2]);

    // Parsear el contenido del mensaje como JSON
    replace(mensaje.begin(), mensaje.end(), '\'', '\"');
    json mensajeJSON = json::parse(mensaje);

    if (mensajeJSON.find("contexto") != mensajeJSON.end() &&
        mensajeJSON["contexto"].find("txtToSearch") != mensajeJSON["contexto"].end() &&
        mensajeJSON["contexto"].find("resultados") != mensajeJSON["contexto"].end()) {

        // Crear el nuevo objeto JSON con el formato especificado
        json newObject;
        newObject["txtToSearch"] = mensajeJSON["contexto"]["txtToSearch"];
        newObject["respuesta"] = json::array();

        json jsonArray = mensajeJSON["contexto"]["resultados"];

        for (const auto& elemento : jsonArray) {
            json elemJson;
            elemJson["archivo"] = elemento["archivo"];
            elemJson["puntaje"] = elemento["puntaje"];
            newObject["respuesta"].push_back(elemJson);
        }

        // Nombre del archivo JSON
        string filename = "cache.json";

        // Cargar datos desde el archivo JSON
        json jsonData = loadJSON(filename);

        // Agregar el nuevo objeto y mantener solo los últimos 4 objetos
        addAndMaintain(jsonData, newObject, memory);

        // Guardar los datos actualizados en el archivo JSON
        saveJSON(filename, jsonData, memory);

        // Imprimir el JSON resultante
        cout << "Archivo json creado con éxito" << endl;

    }
    else {
        cerr << "El JSON proporcionado no tiene las claves esperadas." << endl;
    }

    return 0;
}