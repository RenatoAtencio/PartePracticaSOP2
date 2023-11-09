#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// Funciones para transformar el idx en un hash, y para buscar palabras (un vector de palabras) dentro del hash
void obtenerPalabras(const string &frase, vector<string> &palabras);
void buscarEnIdx(const vector<string> &palabras, int topk, unordered_map<string, vector<pair<string, int>>>& indiceInvertido);
void verificarCantidad (unordered_map<string, int>& interseccion, const int &cantPalabras, vector<string>& textosAceptados);
void crearHash (unordered_map<string, vector<pair<string, int>>>& indiceInvertido, const string& idxFile);
void imprimirHash(const unordered_map<string, vector<pair<string, int>>>& indiceInvertido);

void receiveMessages(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesRead] = '\0';
        cout << "mensaje recibido: '" << buffer << "'\n";
    }
}

/*string receiveMessages(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead;
    stringstream ss;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesRead] = '\0';
        ss << "mensaje recibido: '" << buffer << "'\n";
    }

    return ss.str();
}*/

int main(int argc, char *argv[]) {
    string username = "backend";

    string idxFile = "./file.idx";
    unordered_map<string, vector<pair<string, int>>> indiceInvertido;
    crearHash(indiceInvertido, idxFile);
    vector<string> palabras;
    int topk = 5;

    int clientSocket;
    struct sockaddr_in serverAddr;

    // Crear el socket del cliente
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error al crear el socket del cliente");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Puerto del servidor
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Dirección IP del servidor

    // Conectar al servidor
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al conectar al servidor");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    cout << "Conectado al servidor." << endl;

    // Crear un nuevo hilo para recibir mensajes del servidor
    // hacer que en la funcion recieve messages llame a obtener palabras, buscarEnIdx y ver si funciona
    thread(receiveMessages, clientSocket).detach();

    // Enviar mensajes al servidor
    char message[1024];
    while (true) {
        cout << "Ingrese una frase a buscar: ";
        string frase;
        getline(cin, frase);
        obtenerPalabras(frase, palabras);
        buscarEnIdx(palabras,topk,indiceInvertido);
        string fullMessage = username + " buscando la frase " + frase ;
        send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
        frase.clear();
        palabras.clear();
    }

    // Cerrar el socket del cliente
    close(clientSocket);

    return 0;
}





// imprime el hash, solo para ver que se creó bien
void imprimirHash(const unordered_map<string, vector<pair<string, int>>>& indiceInvertido){
    for (const auto& entrada : indiceInvertido) {
        cout << "Clave: " << entrada.first << endl;
        
        for (const auto& par : entrada.second) {
            cout << "  Subclave: " << par.first << ", Valor: " << par.second << endl;
        }
    }
}

// Crea el hash a partir del idxFile 
void crearHash (unordered_map<string, vector<pair<string, int>>>& indiceInvertido, const string& idxFile){
    ifstream archivo(idxFile);
    if(!archivo.is_open()){
        cerr << "\n- El archivo '" << idxFile << "' no existe!\n\n";
        exit(EXIT_FAILURE);
    }
    string linea, pal, aux;
    int pos;

    while (getline(archivo, linea)) {
        // ecosistema:(texto16.txt;1);(texto17.txt;1);(texto19.txt;1);
        pos = linea.find(":");
        pal = linea.substr(0,pos);
        // pal es cada palabra, ecosistema
        linea = linea.substr(pos+1);
        vector<string> separados;
        stringstream ss(linea);
        string aux;
        while(getline(ss, aux, '(')){
            aux = aux.substr(0, aux.find(")"));
            if(!aux.empty()) separados.push_back(aux);
        }

        for (string &str : separados) {
            int n = str.find(";");
            string nombreTexto = str.substr(0,n);
            string asd = str.substr(n+1);
            int rep = stoi(str.substr(n+1));
            pair<string, int> res (nombreTexto,rep);
            indiceInvertido[pal].push_back(res);  
        }   
    } // termina la creacion del hash
}

// Verifica que textos tienen todas las palabras de la frase ingresada
void verificarCantidad (unordered_map<string, int>& interseccion, const int &cantPalabras, vector<string>& textosAceptados){
    for (const auto& elemento : interseccion){
        if(cantPalabras == elemento.second)  textosAceptados.push_back(elemento.first);
    }
}

// busca las palabras dentro del vector palabras en el hash del indice invertido
// muestra solo <topk> textos
void buscarEnIdx(const vector<string> &palabras, int topk, unordered_map<string, vector<pair<string, int>>>& indiceInvertido){
    unordered_map<string, int> documentosContador; // texto01,sumRep
    unordered_map<string, int> interseccion;
    int cantPalabras = palabras.size();

    for (const auto &palabra : palabras){
        if(indiceInvertido.find(palabra) != indiceInvertido.end()){
            for (const auto &documento : indiceInvertido[palabra]){
                documentosContador[documento.first] += documento.second;
                interseccion[documento.first] += 1;
            }
        }
    }

    // Convertir hashmap en vector para ordernarlo
    vector<pair<string,int>> documentosOrdenados;
    for (const auto &elemento : documentosContador) documentosOrdenados.push_back(elemento);
    // ordenar el vector segun el int del pair
    sort(documentosOrdenados.begin(), documentosOrdenados.end(), [](const auto &a, const auto &b) { // Se implementa una funcion lambda para comparar el INT del par
        return a.second > b.second;
    }); // es la misma lógica que usé para ordenar el conteo de palabras (funciones/archivos.cpp)

    vector<string> textosAceptados;
    verificarCantidad(interseccion, cantPalabras, textosAceptados);

    if (documentosContador.empty() || textosAceptados.empty()) cout << "0) Ninguna coincidencia encontrada!\n";    

    int cont = 1;
    for (const auto &documento : documentosOrdenados){
        if(find(textosAceptados.begin(), textosAceptados.end(), documento.first) != textosAceptados.end()){
            cout << cont << ") " << documento.first << ", " << documento.second << endl;
            cont ++;
            if (cont > topk) break;
        }
    }
}

// a partir de una frase, guarda cada palabra dentro del vector "palabras"
void obtenerPalabras(const string &frase, vector<string> &palabras) {
    istringstream ss(frase);
    string palabra;

    while (ss >> palabra) {
        palabras.push_back(palabra);
    }
}
