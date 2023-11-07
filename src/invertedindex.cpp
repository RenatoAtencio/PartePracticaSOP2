#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

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
            send(memcacheSocket, responseMsg.c_str(), responseMsg.length(), 0);
            close(memcacheSocket);
            cout << "Cliente desconectado" << endl;
            break; // Salir del bucle interior
        }
    }

    // El servidor seguirá esperando nuevas conexiones sin terminar

    return 0;
}
