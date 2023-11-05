
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>

void receiveMessages(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesRead] = '\0';
        std::cout << buffer << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " nombre_de_usuario" << std::endl;
        return EXIT_FAILURE;
    }

    std::string username = argv[1];

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

    std::cout << "Conectado al servidor." << std::endl;

    // Crear un nuevo hilo para recibir mensajes del servidor
    std::thread(receiveMessages, clientSocket).detach();

    // Enviar mensajes al servidor
    char message[1024];
    while (std::cin.getline(message, sizeof(message))) {
        std::string fullMessage = username + " dice: " + message;
        send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
    }

    // Cerrar el socket del cliente
    close(clientSocket);

    return 0;
}
