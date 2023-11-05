#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    // Crear un socket del cliente
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error al crear el socket del cliente" << std::endl;
        return -1;
    }

    // Configurar la dirección del servidor al que queremos conectarnos
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);  // Puerto de ejemplo
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // Dirección IP del servidor

    // Conectar al servidor
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al conectar al servidor" << std::endl;
        close(clientSocket);
        return -1;
    }

    // Enviar datos al servidor
    const char* message = "Hola, servidor!";
    if (send(clientSocket, message, strlen(message), 0) == -1) {
        std::cerr << "Error al enviar datos al servidor" << std::endl;
    }

    // Cerrar el socket del cliente
    close(clientSocket);

    return 0;
}
