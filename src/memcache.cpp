#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>
#include "../include/json.hpp"
#include <chrono>

using json = nlohmann::json;
using namespace std;

// Variables de entorno
const string HOST = "./memcache";
const string FRONT = "./searcher";
const string BACK = "./invertedindex";
const int MEMORYSIZE = 4;

// Funcion para conectar a un servidor, necesita la ip del servidor y el puerto del servidor
int connectToServer(const string& serverIP, int serverPort) {
    // creo el socket para conectarme al server
    int connectionSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (connectionSocket == -1) {
        perror("Error al crear el socket de coneccion");\
            exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());

    if (connect(connectionSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al conectar al servidor");
        close(connectionSocket);
        exit(EXIT_FAILURE);
    }

    // Devuelve el socket de la conexion
    cout << "Conectado al servidor." << endl;
    return connectionSocket;
}

// Enviar msg al servidor, se usa el socket de la conexion
void sendMessage(int connectionSocket, const string& message) {
    send(connectionSocket, message.c_str(), message.length(), 0);
}

// Recivir la respuesta del servidor
string recieveServerMessage(int connectionSocket) {
    char buffer[1024];
    ssize_t bytesRead = recv(connectionSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        perror("Error al recibir datos del servidor");
        close(connectionSocket);
        exit(EXIT_FAILURE);
    }
    else if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
    }
    return buffer;
}

// Comienza un server, con puerto 12345 y ip "127.0.0.1" (Tecnicamente es cualquier ip porque el INADDR_ANY escucha a toda la maquina, el socket es lo que diferenciara a los servers)
int startServer(int& serverSocket, sockaddr_in& serverAddr) {
    // Crear el socket del servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Puerto del servidor
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar el socket a la dirección del servidor
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al enlazar el socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(serverSocket, 5) == -1) {
        perror("Error al escuchar");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    cout << "Esperando conexiones entrantes..." << endl;
    return 1; // Devuelve 1 si se pudo crear el server
}

json loadJSON(const string& filename) {
    json jsonData;
    ifstream file(filename);
    if (file.is_open()) {
        file >> jsonData;
        file.close();
    }
    return jsonData;
}

int main() {
    int memcacheSocket, searcherSocket;
    struct sockaddr_in memcacheAddr, searcherAddr;
    socklen_t clientAddrLen = sizeof(searcherAddr);

    startServer(memcacheSocket, memcacheAddr);

    while (true) {
        // Aceptar la conexión entrante
        searcherSocket = accept(memcacheSocket, (struct sockaddr*)&searcherAddr, &clientAddrLen); // Acepta la conexion del searcher
        if (searcherSocket == -1) {
            perror("Error al aceptar la conexión");
            continue; // Continuar esperando conexiones
        }

        cout << "Cliente conectado" << endl;
        char msg[1024];
        bool success = true;

        while (success) {
            ssize_t msgRead = recv(searcherSocket, msg, sizeof(msg), 0);
            msg[msgRead] = '\0';
            cout << "Mensaje recibido: " << msg << endl; // Mensaje enviado por el searcher 

            // Aqui hacer el parse del msg para obtener el txtToSearch (Todo esto fue hecho con el nlohmann)
            string txtToSearch;
            string jsonString = string(msg);

            // Reemplazar comillas simples por comillas dobles
            replace(jsonString.begin(), jsonString.end(), '\'', '\"');

            // Analizar el string JSON
            json jsonData = json::parse(jsonString);

            // Acceder a la variable 'txtToSearch' en el contexto
            txtToSearch = jsonData["contexto"]["txtToSearch"];

            cout << "Valor de txtToSearch: " << txtToSearch << endl;

            // Aqui ver si el txtToSearch esta en el json del cache
            bool encontrado = false; // Este bool sera el que indica si se encuantra o no la busqueda
            string filename = "data/cache.json";
            ifstream fileStream(filename);
            if (!fileStream.is_open()) {
                cerr << "Error al abrir el archivo JSON." << endl;
                exit(EXIT_FAILURE);
            }
            json jsonArray;
            fileStream >> jsonArray;
            fileStream.close();

            auto start = chrono::high_resolution_clock::now();
            int index = 0;
            for (const auto& elemento : jsonArray) {
                if (elemento["Busqueda"] == txtToSearch) {
                    // Se encontró una coincidencia
                    encontrado = true;
                    index++;
                }
            }
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start).count(); // El tiempo lo puse solo en lo que se demora en buscar

            // recorrer el json, comparar 'busqueda' a txtToSearch
            // Si lo encuentra entonces encontrado = true y devolver el elemnto

            if (encontrado == true) { // Si se encontro la busqueda en cache
                // Tomar lo del json y parsearlo en el tipo de msg que pide
                string origen, destino, tiempo, ori, isFound, resultado;
                origen = HOST;
                destino = FRONT;
                tiempo = to_string(duration);
                ori = "MEMCACHE";
                isFound = "true";
                resultado = jsonArray[index]["Respuesta"].dump();

                string commandResp = "python3 src/format.py 2 " + origen + " " + destino + " " + tiempo + " " + ori + " " + isFound + " '" + resultado + "'";
                int successResp = system(commandResp.c_str());
                string msgToFront;
                if (successResp == 0) {
                    ifstream readMsg;
                    readMsg.open("data/msg.txt");
                    string line;
                    while (getline(readMsg, line)) {
                        msgToFront += line; // Agregar cada línea al contenido
                    }
                }
                else {
                    cout << "No se pudo llamar al programa externo para crear el msg";
                    exit(EXIT_FAILURE);
                }
                send(searcherSocket, msgToFront.c_str(), msgToFront.length(), 0);
            }
            else { // Si no se encontro en cache se debe enviar el msg al indice invertido
                string serverIP = "127.0.0.1";  // Dirección IP del servidor
                int invIndexPort = 12346;       // Puerto del servidor de inverted index
                int index_socket = connectToServer(serverIP, invIndexPort); //Conectar al servidor del index, si no se pudo se termina la ejecucion

                string commandMsg = "python3 src/format.py 1 " + FRONT + " " + BACK + " " + txtToSearch; // Llamo al prog externo para crear el formato del msg
                int successMsg = system(commandMsg.c_str());
                string msgToIndex; // msg que se enviara al index
                if (successMsg == 0) {
                    ifstream readMsg;
                    readMsg.open("data/msg.txt");
                    string line;
                    while (getline(readMsg, line)) {
                        msgToIndex += line; // Agregar cada línea al contenido
                    }
                }
                else {
                    cout << "No se pudo llamar al programa externo para crear el msg";
                    exit(EXIT_FAILURE);
                }

                sendMessage(index_socket, msgToIndex); // se envia el msg al index
                string resp = recieveServerMessage(index_socket); // respuesta del index
                // parcearla para agrarla al index
                // llamar al prog externo

                cout << "La respuesta del index es: " << resp << endl;
                close(index_socket);
                send(searcherSocket, resp.c_str(), resp.length(), 0); // envia la respuesta del index al searcher (La respuesta del index ya deberia de estar con el formato correcto)
            }
        }
    }
    return 0;
}
