#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Crear el socket del cliente (Programa 3)
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error al crear el socket del cliente (Programa 3)");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Puerto del Programa 2
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Dirección IP del servidor (localhost)

    // Conectar al servidor
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al conectar al servidor (Programa 2)");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // Enviar credenciales al servidor (Programa 2)
    string credentials = "invertedIndex,invertedIndex"; // Credenciales de Programa 3
    send(clientSocket, credentials.c_str(), credentials.length(), 0);
    cout << "Programa 3 - Credenciales enviadas al servidor" << endl;

    // Resto del código para la comunicación con el servidor

    close(clientSocket);
    return 0;
}
