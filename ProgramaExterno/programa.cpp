#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

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
void saveJSON(const string& filename, const json& data) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << data.dump(4);  // La función dump(4) formatea la salida con 4 espacios de indentación.
        outFile.close();
    } else {
        cerr << "No se pudo abrir el archivo para escritura." << endl;
    }
}

// Función para agregar un nuevo objeto JSON y mantener solo los últimos 4 objetos
void addAndMaintain(json& data, const json& newObject) {
    data.push_back(newObject);
    if (data.size() > 4) {
        data.erase(data.begin());
    }
}

int main() {
    // Nombre del archivo JSON
    string filename = "data.json";

    // Cargar datos desde el archivo JSON
    json jsonData = loadJSON(filename);

    // Crear un nuevo objeto JSON
    json newObject;
    newObject["mi_lista"] = nlohmann::json::array();
    newObject["id"] = 3;
    newObject["nombre"] = "Ejemplo 3";
    newObject["dato"] = "Información 3";
    newObject["mi_lista"].push_back("elemento1");
    newObject["mi_lista"].push_back("elemento2");

    // Agregar el nuevo objeto y mantener solo los últimos 4 objetos
    addAndMaintain(jsonData, newObject);

    // Guardar los datos actualizados en el archivo JSON
    saveJSON(filename, jsonData);

    // Imprimir el JSON resultante
    //cout << jsonData.dump(4) << endl;

    cout<<"Archivo data.json generado correctamente "<<endl;

    return 0;
}