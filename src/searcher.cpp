#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

// Función para conectar al servidor
int connectToServer(const string& serverIP, int serverPort) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error al crear el socket del cliente");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al conectar al servidor");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    cout << "Conectado al servidor." << endl;
    return clientSocket;
}

// Enviar msg al servidor (el msg pasa como string)
void sendMessage(int clientSocket, const string& message) {
    send(clientSocket, message.c_str(), message.length(), 0);
}

// Recivir la respuesta del servidor
void receiveAndDisplayMessage(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        perror("Error al recibir datos del servidor");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    else if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        cout << "Respuesta del servidor:\n" << buffer << endl;
    }
}

int main() {
    string serverIP = "127.0.0.1"; // Dirección IP del servidor
    int serverPort = 12345;       // Puerto del servidor
    int clientSocket = connectToServer(serverIP, serverPort); //Conectar al servidor, si no se pudo se termina la ejecucion


    string msg;
    bool again = true;
    while (again) {
        cout << "ingrese su busqueda: ";
        getline(cin,msg);
        sendMessage(clientSocket, msg);
        receiveAndDisplayMessage(clientSocket);
    }

    //close(clientSocket);
    return 0;
}
