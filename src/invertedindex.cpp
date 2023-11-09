#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/json.hpp"
#include <chrono>
#include <fstream>

using json = nlohmann::json;
using namespace std;

// Variables de entorno
const string FROM = "./searcher";
const string TO = "./memcache";
const string _FILE = "inverted_index_file.idx"; // (Solo FILE dabe error, colisionaba con algo parece)
const int TOPK = 4;

int startServer(int& invIndexSocket, sockaddr_in& invIndexAddr, sockaddr_in& memcacheAddr) {
    // Crear el socket del servidor
    invIndexSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (invIndexSocket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&invIndexAddr, 0, sizeof(invIndexAddr));
    invIndexAddr.sin_family = AF_INET;
    invIndexAddr.sin_port = htons(12346);
    invIndexAddr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar el socket a la dirección del servidor
    if (bind(invIndexSocket, (struct sockaddr*)&invIndexAddr, sizeof(invIndexAddr)) == -1) {
        perror("Error al enlazar el socket");
        close(invIndexSocket);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(invIndexSocket, 5) == -1) {
        perror("Error al escuchar");
        close(invIndexSocket);
        exit(EXIT_FAILURE);
    }
    cout << "Esperando conexiones entrantes..." << endl;
    return 1;
}

int main() {
    int invIndexSocket, memcacheSocket;
    struct sockaddr_in invIndexAddr, memcacheAddr;
    socklen_t clientAddrLen = sizeof(memcacheAddr);

    startServer(invIndexSocket, invIndexAddr, memcacheAddr);

    while (true) {
        // Aceptar la conexión entrante
        memcacheSocket = accept(invIndexSocket, (struct sockaddr*)&memcacheAddr, &clientAddrLen);
        if (memcacheSocket == -1) {
            perror("Error al aceptar la conexión");
            continue; // Continuar esperando conexiones
        }

        cout << "Cliente conectado" << endl;
        char msg[1024];
        bool success = true;

        while (success) {
            ssize_t msgRead = recv(memcacheSocket, msg, sizeof(msg), 0);
            msg[msgRead] = '\0';
            cout << "Mensaje recibido: " << msg << endl;
            string responseMsg = "El mensaje recibido fue: " + string(msg);

            // Aqui hacer el parse del msg para obtener el txtToSearch (Todo esto fue hecho con el nlohmann)
            string txtToSearch;
            string jsonString = string(msg);

            // Reemplazar comillas simples por comillas dobles
            replace(jsonString.begin(), jsonString.end(), '\'', '\"');

            // Analizar el string JSON
            json jsonData = json::parse(jsonString);

            // Acceder a la variable 'txtToSearch' en el contexto
            txtToSearch = jsonData["contexto"]["txtToSearch"];

            auto start = chrono::high_resolution_clock::now();
            string commandSearch = "./buscador data/ file.idx " + txtToSearch + " 4";
            int successSearch = system(commandSearch.c_str());
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            string msg;
            if (successSearch == 0) {
                ifstream readMsg;
                readMsg.open("data/invIndexOutput.txt");
                string line;
                while (getline(readMsg, line)) {
                    msg += line; // Agregar cada línea al contenido
                }

                string origen, destino, tiempo, ori, isFound, resultado;
                origen = FROM;
                destino = TO;
                tiempo = to_string(duration);
                ori = "BACKEND";
                isFound = "true";
                resultado = msg;

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
                send(memcacheSocket, msgToFront.c_str(), msgToFront.length(), 0);
                close(memcacheSocket);
                cout << "Cliente desconectado" << endl;
            }
            else {
                cout << "No se pudo llamar al buscador" << endl;
                exit(EXIT_FAILURE);
            }
            break; // Salir del bucle interior
        }
    }

    // El servidor seguirá esperando nuevas conexiones sin terminar

    return 0;
}
