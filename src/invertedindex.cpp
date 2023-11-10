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

// Toma el mensaje y devuelve la busqueda hecha por el usuario
string getSearchFromMsg(string msg) {
    string txtToSearch;
    string jsonString = string(msg);

    // Carga el string como un json (hace mas facil el tomar la variable txtToSearch)
    replace(jsonString.begin(), jsonString.end(), '\'', '\"');
    json jsonData = json::parse(jsonString);
    txtToSearch = jsonData["contexto"]["txtToSearch"];

    cout << "Busqueda hecha por el cliente: " << txtToSearch << endl;
    return (txtToSearch);
}

/*Genera el string del mensaje de respuesta, el formato es
    mensaje = {
        "origen": "origen",
        "destino": "destino",
        "contexto": {
            "tiempo": "tiempo(ns)",
            "ori": "origen",
            "isFound": "ResultadoBusqueda",
            "resultados" : [
                {"archivo": "texto1", "puntaje": "cant1"},
                {"archivo": "texto2", "puntaje": "cant2"},
                {"archivo": "texto3", "puntaje": "cant3"},
                {"archivo": "texto4", "puntaje": "cant4"},
                {"archivo": "texto5", "puntaje": "cant5"}
            ]
        }
    }
*/
string generarMsgRespuesta(string origen, string destino, string txtToSearch, string tiempo, string ori, string resultado) {
    string commandResp = "python3 src/format.py 2 " + origen + " " + destino + " '" + txtToSearch + "' " + tiempo + " " + ori + " '" + resultado + "'";
    int successResp = system(commandResp.c_str());
    string msgRespuesta;
    if (successResp == 0) {
        ifstream readMsg;
        readMsg.open("data/msg.txt");
        string line;
        while (getline(readMsg, line)) {
            msgRespuesta += line; // Agregar cada línea al contenido
        }
    }
    else {
        cout << "No se pudo llamar al programa externo para crear el msg";
        exit(EXIT_FAILURE);
    }
    return (msgRespuesta);
}

int callInvertedIndex(string txtToSearch, int topK) {
    string commandSearch = "./buscador data/archivosOut/ file.idx '" + txtToSearch + "' " + to_string(topK);
    int successSearch = system(commandSearch.c_str());
    return (successSearch);
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

            // Se toma la busqueda hecha por el usuario desde el msg (que esta en formato json)
            string txtToSearch = getSearchFromMsg(msg);

            // Llamo al inv index que genera el invIndexOutput.txt con el resultado de la busqueda, tambien calculo el tiempo de ejecucion
            auto start = chrono::high_resolution_clock::now();
            int successSearch = callInvertedIndex(txtToSearch, TOPK);
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

                // Generar el msg de respuesta
                string origen, destino, tiempo, ori, resultado;
                origen = FROM;
                destino = TO;
                tiempo = to_string(duration);
                ori = "BACKEND";
                resultado = msg;
                string msgToFront = generarMsgRespuesta(origen, destino, txtToSearch, tiempo, ori, resultado);

                // Mandar el msg a la cache y cerrar la conexion con la cache
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
    return 0;
}
