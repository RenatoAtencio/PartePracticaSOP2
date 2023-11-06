#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

int startServer(int& serverSocket, sockaddr_in& serverAddr, sockaddr_in& clientAddr) {
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
    return 1;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    startServer(serverSocket, serverAddr, clientAddr);

    while (true) {
        // Aceptar la conexión entrante
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Error al aceptar la conexión");
            continue; // Continuar esperando conexiones
        }

        cout << "Cliente conectado" << endl;
        send(clientSocket, "Inicio de sesion correcto", 30, 0);
        char msg[1024];
        ssize_t bytesRead;
        bool success = true;

        while (success) {
            ssize_t msgRead = recv(clientSocket, msg, sizeof(msg), 0);
            msg[msgRead] = '\0';


            cout << "Mensaje recivido: " << msg << endl;
            if (string(msg) == "salir") {
                close(clientSocket);
                cout << "Cliente desconectado" << endl;
                success = false;
            } else {
                string responceMsg = "El msg recivido fue: " + string(msg);
                send(clientSocket, responceMsg.c_str(), responceMsg.length(), 0);
            }
        }
    }

    return 0;
}
