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

/* Genera el string del mensaje de busqueda, el formato es
    mensaje = {
        "origen" : "origen",
        "destino" : "destino",
        "contexto" : {
            "txtToSearch" : "busqueda"
        }
    }
*/
string generarMsgBusqueda(string origen, string destino, string busqueda) {
    string commandMsg = "python3 src/format.py 1 " + origen + " " + destino + " '" + busqueda + "'";
    int successMsg = system(commandMsg.c_str());
    string msg;
    if (successMsg == 0) {
        ifstream readMsg;
        readMsg.open("data/msg.txt");
        string line;
        while (getline(readMsg, line)) {
            msg += line; // Agregar cada línea al contenido
        }
        return (msg);
    }
    else {
        cout << "No se pudo llamar al programa externo para crear el msg";
        exit(EXIT_FAILURE);
    }
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

// Carga un json
json loadJSON(const string& filename) {
    json jsonData;
    ifstream file(filename);
    if (file.is_open()) {
        file >> jsonData;
        file.close();
    }
    return jsonData;
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

// Ve si la busqueda esta en la memoria
bool verifyInCacheMemory(string txtToSearch, int& index, json& jsonArray) {
    string filename = "data/cache.json";
    ifstream fileStream(filename);
    if (!fileStream.is_open()) {
        cerr << "No existe o no se pudo abrir cache.json" << endl;
        return(false);
    }
    fileStream >> jsonArray;
    fileStream.close();

    for (const auto& elemento : jsonArray) {
        if (elemento["txtToSearch"] == txtToSearch) {
            return (true);
        }
        index++;
    }
    return (false);
}

/*Genera el string del mensaje de respuesta, el formato es
    mensaje = {
        "origen": "origen",
        "destino": "destino",
        "contexto": {
            "tiempo": "tiempo(ns)",
            "ori": "origen",
            "isFound": "ResultadoBusqueda",
            "txtToSearch": "busqueda",
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

int main() {
    string HOST,FRONT,BACK;
    int MEMORYSIZE;
    string jsonEnvFileName = "config/memcache_env.json";
    ifstream jsonEnv(jsonEnvFileName);
    if (jsonEnv.is_open()) {
        json jsonData;
        jsonEnv >> jsonData;
        jsonEnv.close();
        HOST = jsonData["HOST"];
        FRONT = jsonData["FRONT"];
        BACK = jsonData["BACK"];
        MEMORYSIZE = jsonData["MEMORYSIZE"];
    }

    // Elimino el cache al correr el cache
    string jsonFileName = "data/cache.json";
    ifstream file(jsonFileName);
    if (file.good()) {
        file.close();
        remove(jsonFileName.c_str());
    }

    int memcacheSocket, searcherSocket;
    struct sockaddr_in memcacheAddr, searcherAddr;
    socklen_t clientAddrLen = sizeof(searcherAddr);

    // Se prende el server de la memcache
    startServer(memcacheSocket, memcacheAddr);

    while (true) {
        // Aceptar la conexión entrante (El searcher)
        searcherSocket = accept(memcacheSocket, (struct sockaddr*)&searcherAddr, &clientAddrLen);
        if (searcherSocket == -1) {
            perror("Error al aceptar la conexión");
            continue; // Continuar esperando conexiones
        }

        cout << "Cliente conectado" << endl;
        char msg[1024];
        bool success = true;

        while (success) {
            ssize_t msgRead = recv(searcherSocket, msg, sizeof(msg), 0); // Recibe el msg enviado por el searcher
            msg[msgRead] = '\0';
            cout << "Mensaje recibido: " << msg << endl;

            // Se toma la busqueda hecha por el usuario desde el msg (que esta en formato json)
            string txtToSearch = getSearchFromMsg(msg);

            // Buscar en memoria la busqueda, tambien se calcula el tiempo en caso de que si este en memoria
            int index = 0;
            json jsonArray; // Mensaje como json
            jsonArray.clear();
            auto start = chrono::high_resolution_clock::now();
            bool encontrado = verifyInCacheMemory(txtToSearch, index, jsonArray); // Devuelve true si esta en memoria, falso si no
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start).count(); // El tiempo lo puse solo en lo que se demora en buscar

            if (encontrado == true) { // Encontro la busqueda en la memoria
                string origen, destino, tiempo, ori, isFound, resultado;
                origen = HOST;
                destino = FRONT;
                tiempo = to_string(duration);
                ori = "MEMCACHE";
                resultado = jsonArray[index]["resultados"].dump();
                string msgToFront = generarMsgRespuesta(origen, destino, txtToSearch, tiempo, ori, resultado); // Generar msg de respuesta

                // Manda el msg de respuesta al searcher
                send(searcherSocket, msgToFront.c_str(), msgToFront.length(), 0);
                ssize_t msgRead = recv(searcherSocket, msg, sizeof(msg), 0); // Recibe el msg enviado por el searcher
                msg[msgRead] = '\0';
                if (string(msg) == "s") {
                    close(searcherSocket);
                    cout << "Se desconecto el cliente" << endl;
                    success = false;
                }else{
                    continue;
                }
            }
            else { // Si no se encontro en memoria
                string serverIP = "127.0.0.1";  // Dirección IP del servidor
                int invIndexPort = 12346;       // Puerto del servidor de inverted index
                int index_socket = connectToServer(serverIP, invIndexPort); // Conectar al servidor del index

                string msgToIndex = generarMsgBusqueda(FRONT, BACK, txtToSearch); // Genera el msg de busqueda para enviarlo al index

                // Enviar y recibir respuesta del index
                sendMessage(index_socket, msgToIndex);
                string resp = recieveServerMessage(index_socket);

                // Agregar la respuesta del index a la memoria 
                string commandAddToJson = "./actualizarJson '" + resp + "' " + to_string(MEMORYSIZE);
                int successAdd = system(commandAddToJson.c_str());
                if (successAdd == 0) {
                    cout << "Ultima busqueda agregada al cache" << endl;
                }
                else {
                    cout << "No se puedo llamar al programa externo" << endl;
                    exit(EXIT_FAILURE);
                }

                cout << "La respuesta del index es: " << resp << endl;
                close(index_socket);
                send(searcherSocket, resp.c_str(), resp.length(), 0); // envia la respuesta del index al searcher (La respuesta del index ya deberia de estar con el formato correcto)
                ssize_t msgRead = recv(searcherSocket, msg, sizeof(msg), 0); // Recibe el msg enviado por el searcher
                msg[msgRead] = '\0';
                if (string(msg) == "s") {
                    close(searcherSocket);
                    cout << "Se desconecto el cliente" << endl;
                    success = false;
                }else{
                    continue;
                }
            }
        }
    }
    return 0;
}
