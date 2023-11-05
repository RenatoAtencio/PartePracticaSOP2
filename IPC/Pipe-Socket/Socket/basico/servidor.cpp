#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    // Crear un socket del servidor
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket del servidor" << std::endl;
        return -1;
    }

    // Configurar la dirección del servidor
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);  // Puerto de ejemplo
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Vincular el socket a la dirección y el puerto
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al vincular el socket" << std::endl;
        close(serverSocket);
        return -1;
    }

    // Escuchar por conexiones entrantes
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error al escuchar por conexiones entrantes" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Servidor esperando conexiones..." << std::endl;

    // Aceptar una conexión entrante
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (clientSocket == -1) {
        std::cerr << "Error al aceptar la conexión" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Cliente conectado" << std::endl;

    // Leer datos del cliente
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        std::cerr << "Error al recibir datos del cliente" << std::endl;
    } else {
        buffer[bytesRead] = '\0';
        std::cout << "Mensaje del cliente: " << buffer << std::endl;
    }

    // Cerrar los sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}
