// Programa 2 - Servidor
// Recibe mensajes de Programa 1 y Programa 3 después de la autenticación

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include <unordered_map>

using namespace std;

// Estructura para almacenar las credenciales de los clientes
struct ClientCredentials {
    string username;
    string password;
};

// Base de datos de credenciales (nombre de usuario y contraseña)
vector<ClientCredentials> clientDatabase = {
    {"searcher", "searcher"}, // Credenciales de Programa 1
    {"invertedIndex", "invertedIndex"}, // Credenciales de Programa 3
};

// Estructura para almacenar los identificadores de cliente y sus sockets
unordered_map<string, int> clientSocketsMap;

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;
    socklen_t clientAddrLen;

    // Crear el socket del servidor (Programa 2)
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Puerto del Programa 2
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
    cout << "Programa 2 - Esperando conexiones entrantes..." << endl;

    while (true) {
        struct sockaddr_in clientAddr;
        clientAddrLen = sizeof(clientAddr);

        // Aceptar una conexión entrante (Programa 1 o Programa 3)
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Error al aceptar la conexión de un cliente");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        cout << "Programa 2 - Cliente conectado" << endl;

        char msg[1024];
        ssize_t bytesRead;

        while (true) {
            // Recibir el mensaje del cliente (Programa 1 o Programa 3)
            bytesRead = recv(clientSocket, msg, sizeof(msg), 0);
            if (bytesRead <= 0) {
                // Cliente desconectado
                close(clientSocket);
                cout << "Programa 2 - Cliente desconectado" << endl;
                // Eliminar el identificador del cliente si está en el mapa
                for (auto it = clientSocketsMap.begin(); it != clientSocketsMap.end(); ++it) {
                    if (it->second == clientSocket) {
                        clientSocketsMap.erase(it);
                        break;
                    }
                }
                break;
            }
            msg[bytesRead] = '\0';

            // Autenticación del cliente
            bool authenticated = false;
            for (const ClientCredentials& credentials : clientDatabase) {
                if (strcmp(msg, (credentials.username + "," + credentials.password).c_str()) == 0) {
                    authenticated = true;
                    cout << "Programa 2 - Cliente autenticado como " << credentials.username << endl;
                    // Asignar el identificador del cliente al socket
                    clientSocketsMap[credentials.username] = clientSocket;
                    break;
                }
            }

            if (!authenticated) {
                cout << "Programa 2 - Cliente no autenticado. Cerrando conexión..." << endl;
                close(clientSocket);
            }
        }
    }

    return 0;
}
