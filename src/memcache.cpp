#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

int main() {
    int memcacheSocket, searcherSocket;
    struct sockaddr_in memcacheAddr, searcherAddr;
    socklen_t clientAddrLen = sizeof(searcherAddr);

    startServer(memcacheSocket, memcacheAddr);

    while (true) {
        // Aceptar la conexión entrante
        searcherSocket = accept(memcacheSocket, (struct sockaddr*)&searcherAddr, &clientAddrLen);
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
            cout << "Mensaje recibido: " << msg << endl;
            string responseMsg = "El mensaje recibido fue: " + string(msg);
            if (string(msg) == "Salir") {
                send(searcherSocket, responseMsg.c_str(), responseMsg.length(), 0);
                close(searcherSocket);
                cout << "Cliente desconectado" << endl;
                break; // Salir del bucle interior
            }
            else if (string(msg) == "Conectar Index") {
                string serverIP = "127.0.0.1";  // Dirección IP del servidor
                int invIndexPort = 12346;       // Puerto del servidor de inverted index
                int index_socket = connectToServer(serverIP, invIndexPort); //Conectar al servidor, si no se pudo se termina la ejecucion
                sendMessage(index_socket, msg);
                string resp = recieveServerMessage(index_socket);
                cout << "La respuesta del index es: " << resp << endl;
                close(index_socket);
                send(searcherSocket, resp.c_str(), resp.length(), 0);
            }
            else {
                send(searcherSocket, responseMsg.c_str(), responseMsg.length(), 0);
            }
        }
    }

    // El servidor seguirá esperando nuevas conexiones sin terminar

    return 0;
}
