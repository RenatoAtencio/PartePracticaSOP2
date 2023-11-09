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
        outFile << data.dump(memory);  // La función dump(4) formatea la salida con 4 espacios de indentación.
        outFile.close();
    } else {
        cerr << "No se pudo abrir el archivo para escritura." << endl;
    }
}

// Función para agregar un nuevo objeto JSON y mantener solo los últimos 4 objetos
void addAndMaintain(json& data, const json& newObject,int memory) {
    data.push_back(newObject);
    if (data.size() > static_cast<size_t>(memory)) {
        data.erase(data.begin());
    }
}

int main(int argc, char *argv[]) {
    /* la entrada por consola debe ser asi:
    ./programa 'mensaje={"origen":"./invertedindex","destino":"./memcache","contexto":{"tiempo":"1000ns","ori":"backend","isFound":true,"busqueda":"",
    "resultados":[{"archivo":"file-text1.txt","puntaje":"122"},{"archivo":"file-text20.txt","puntaje":"34"},{"archivo":"file-text3.txt","puntaje":"10"},{"archivo":"file-text2.txt","puntaje":"2"}]}}'
    */
    // Verificar si se proporciona un mensaje como argumento de línea de comandos
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " mensaje memory" << endl;
        return 1;
    }

    // Obtener el mensaje desde la línea de comandos
    string mensaje = argv[1];
    int memory = atoi(argv[2]);

    // Encontrar la posición de '=' para extraer el contenido del mensaje
    size_t pos = mensaje.find('=');
    if (pos == string::npos) {
        cerr << "Formato de mensaje no válido." << endl;
        return 1;
    }

    // Obtener el contenido del mensaje
    string jsonContent = mensaje.substr(pos + 1);

    // Parsear el contenido del mensaje como JSON
    json mensajeJSON = json::parse(jsonContent);

    // Crear el nuevo objeto JSON con el formato especificado
    json newObject;
    newObject["Busqueda"] = mensajeJSON["contexto"]["busqueda"];
    newObject["Resultado"] = json::array();

    for (const auto& resultado : mensajeJSON["contexto"]["resultados"]) {
        json fileResult;
        fileResult[resultado["archivo"].get<string>()] = resultado["puntaje"];
        newObject["Resultado"].push_back(fileResult);
    }

    // Nombre del archivo JSON
    string filename = "data.json";

    // Cargar datos desde el archivo JSON
    json jsonData = loadJSON(filename);

    // Agregar el nuevo objeto y mantener solo los últimos 4 objetos
    addAndMaintain(jsonData, newObject, memory);

    // Guardar los datos actualizados en el archivo JSON
    saveJSON(filename, jsonData, memory);

    // Imprimir el JSON resultante
    //cout << jsonData.dump(4) << endl;
    cout<<"Archivo json generado de manera exitosa "<<endl;

    return 0;
}