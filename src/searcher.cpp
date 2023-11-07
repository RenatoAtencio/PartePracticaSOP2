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
string recieveServerMessage(int connectionSocket){
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

int main() {
    string serverIP = "127.0.0.1";  // Dirección IP del servidor
    int memcachePort = 12345;       // Puerto del servidor memcache
    int searcher_memcache_Socket = connectToServer(serverIP, memcachePort); //Socket entre el searcher y el memcache (Con este uno envia y recive msg del o al memcache)

    string msg;
    bool again = true;
    while (again) {
        cout << "ingrese su busqueda: ";
        getline(cin, msg);
        sendMessage(searcher_memcache_Socket, msg);
        string response = recieveServerMessage(searcher_memcache_Socket);
        cout << "Mensaje de respuesta del servidor: " << response << endl;
        if (msg == "Salir") {
            cout << "Se ha desconectado" << endl;
            close(searcher_memcache_Socket);
            exit(EXIT_FAILURE);
        }
    }
    //close(searcherSocket);
    return 0;
}
