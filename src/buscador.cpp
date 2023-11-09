#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <bits/stdc++.h> 

using namespace std;

vector<pair<string, int>> separarVectorResults(string vectorResults);
unordered_map<string, vector<pair<string, int>>>  makeUnorderedMap(string pathFile);


int main(int argc, char* argv[]) {
    unordered_map<string, vector<pair<string, int>>> invertedIndex;
    unordered_map<string, vector<pair<string, int>>> allSearchResults;
    unordered_map<string, vector<pair<string, int>>> goodSearchResults;
    if (argc < 2) {
        cerr << "No ingreso suficientes argumentos" << endl;
        return 1;
    }

    // Paso el .idx a invertedIndex
    string indexFilePath = string(argv[1]);
    string indexFileName = string(argv[2]);
    string txtToSearch = string(argv[3]);
    invertedIndex = makeUnorderedMap(indexFilePath + indexFileName);

    int topk = atoi(argv[4]);

    // Creo la estructura de la busqueda
    istringstream inputStream(txtToSearch);
    string word;
    int searchInputSize = 0;

    while (getline(inputStream, word, ' ')) {
        searchInputSize++;
        if (invertedIndex.find(word) != invertedIndex.end()) {
            // Acceder al vector de pares asociado a la clave y recorrerlo
            for (const pair<string, int>& entry : invertedIndex[word]) {
                allSearchResults[entry.first].emplace_back(word, entry.second);
                // cout << entry.first << " " << word << " " << entry.second << endl;
            }
        }
        else {
            cout << "No se encontro concidencias de la palabra: " << word << endl;
        }
    }

    // Paso todos los resultados que contengan la misma cant de palabras
    // Por ahora muestra solo los que tengan todas las palabras
    for (const auto& entry : allSearchResults) {
        int vectorSize = entry.second.size();
        if (vectorSize == searchInputSize) {
            goodSearchResults[entry.first] = entry.second;
        }
    }

    // Sumar las cantidades
    vector<pair<string, int>> endResults;
    cout << "Respuesta: ";
    if (goodSearchResults.size() == 0) { // si no hay coincidencias crea la lista vacia
        ofstream outputFile("data/invIndexOutput.txt");
        if (outputFile.is_open()) {
            outputFile << "[]";
            outputFile.close();
            cout << "Archivo 'invIndexOutput' creado exitosamente." << std::endl;
        }
        else {
            cerr << "Error al abrir el archivo 'invIndexOutput'." << std::endl;
        }
    }
    else {
        for (const auto& entry : goodSearchResults) {
            int sum = 0;
            const vector<pair<string, int>>& pairs = entry.second;
            for (const pair<string, int>& keyValue : pairs) {
                int value = keyValue.second;
                sum = sum + value;
            }
            endResults.emplace_back(entry.first, sum);
        }

        // Ver como funciona    
        sort(endResults.begin(), endResults.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
            });

        // Ahora, imprime los resultados ordenados.
        string jsonOutput = "[";
        for (const pair<string, int>& result : endResults) {
            topk--;
            if (topk < 0) {
                break;
            }
            // Agregar para imprimir el vector de mayor a menor usando el int del pair y respetando el topk < 0
            string fileName = result.first;
            int value = result.second;
            jsonOutput += "{ \"archivo\": \"" + fileName + "\", \"puntaje\": " + to_string(value) + " }";

            // Agregar una coma si no es el último elemento
            if (topk > 0) {
                jsonOutput += ", ";
            }
        }
        jsonOutput += "]";

        ofstream outputFile("data/invIndexOutput.txt");
        if (outputFile.is_open()) {
            outputFile << jsonOutput;
            outputFile.close();
            cout << "Archivo 'invIndexOutput' creado exitosamente." << std::endl;
        }
        else {
            cerr << "Error al abrir el archivo 'invIndexOutput'." << std::endl;
        }
    }
    return 0;
}

unordered_map<string, vector<pair<string, int>>> makeUnorderedMap(string pathFile) {
    unordered_map<string, vector<pair<string, int>>> invertedIndex;
    ifstream inputFile(pathFile);
    string line;
    // line seria una linea del archivo .idx
    while (getline(inputFile, line)) {
        istringstream lineStream(line);
        string word, vectorResults;
        // Separo la linea en : y le doy los valores de word = palabra y vectorResults = vector
        while ((getline(lineStream, word, ':')) && (getline(lineStream, vectorResults, ':'))) {
            invertedIndex[word] = separarVectorResults(vectorResults);
        }
    }
    inputFile.close();
    return invertedIndex;
}

vector<pair<string, int>> separarVectorResults(string vectorResults) {
    vector<pair<string, int>> outputVector;
    // Le quito el ultimo ; que quedaba sobrando
    vectorResults = vectorResults.substr(0, vectorResults.size() - 1);
    istringstream lineStreamVector(vectorResults);
    string vector;
    while (getline(lineStreamVector, vector, ';')) {
        // Le quito los ( )
        vector = vector.substr(1, vector.size() - 2);
        istringstream lineStreamResult(vector);
        string file, amount;
        while ((getline(lineStreamResult, file, ',')) && (getline(lineStreamResult, amount, ','))) {
            outputVector.emplace_back(file, stoi(amount));
        }
    }
    return outputVector;

}