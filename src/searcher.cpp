#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>
#include <limits>
#include "../include/json.hpp"

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
        perror("Error al conectar al servidor cache");
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

void imprimirResultado(string respuesta) {
    json jsonData = json::parse(respuesta);
    string tiempo = jsonData["contexto"]["tiempo"];
    string origen = jsonData["contexto"]["ori"];
    json jsonArray = jsonData["contexto"]["resultados"];

    cout << "Respuesta: (Tiempo: " << tiempo << "ns, Origen: " << origen << ")" << endl;
    int index = 1;
    if (jsonArray.empty()) {
        cout << "   La busqueda no aparece en ningun archivo" << endl;
    }
    else {
        for (const auto& elemento : jsonArray) {
            cout << "   " << index << ") " << elemento["archivo"] << " " << elemento["puntaje"] << endl;
            index++;
        }
    }
}

int main() {
    string FROM,TO;
    string jsonFileName = "config/searcher_env.json";
    ifstream file(jsonFileName);
    if (file.is_open()) {
        json jsonData;
        file >> jsonData;
        file.close();
        FROM = jsonData["FROM"];
        TO = jsonData["TO"];
    }

    string serverIP = "127.0.0.1";  // Dirección IP del servidor
    int memcachePort = 12345;       // Puerto del servidor memcache
    int searcher_memcache_Socket = connectToServer(serverIP, memcachePort); //Socket entre el searcher y el memcache (Con este uno envia y recive msg del o al memcache)

    cout << "Buscador basado en indice invertido (" << getpid() << ")" << endl;

    bool again = true;
    while (again) {
        string userInput;
        cout << "Escriba texto a buscar: ";
        getline(cin, userInput);

        // Generar y enviar msg a memcache
        string msg = generarMsgBusqueda(FROM, TO, userInput);
        sendMessage(searcher_memcache_Socket, msg);

        // Recibir e imprimir respuesta 
        string response = recieveServerMessage(searcher_memcache_Socket);
        imprimirResultado(response);

        // Salir
        char salir;
        cout << "Desea salir? (s/n): ";
        cin >> salir;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); //Limpia el buffer para que al poner la respuesta no mande algo vacio al server
        if (salir == 's') {
            sendMessage(searcher_memcache_Socket, "s");
            cout << "Se ha desconectado" << endl;
            exit(EXIT_FAILURE);
        }
        else {
            sendMessage(searcher_memcache_Socket, "n");
        }

    }
    return 0;
}
