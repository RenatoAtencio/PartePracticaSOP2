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

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al conectar al servidor");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    cout << "Conectado al servidor." << endl;

    return clientSocket;
}

// Función para enviar un mensaje al servidor
void sendMessage(int clientSocket, const string& message) {
    send(clientSocket, message.c_str(), message.length(), 0);
}

// Función para recibir y mostrar un mensaje del servidor
void receiveAndDisplayMessage(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        perror("Error al recibir datos del servidor");
        close(clientSocket);
        exit(EXIT_FAILURE);
    } else if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        cout << "Respuesta del servidor:\n" << buffer << endl;
    }
}

void login (int clientSocket){
    string username, password;
    cout << "Ingrese su nombre de usuario: ";
    cin >> username;
    cout << "Ingrese su contraseña: ";
    cin >> password;
    string credentials = username + " " + password;
    sendMessage(clientSocket, credentials);
    char response[1024];
    ssize_t bytesRead = recv(clientSocket, response, sizeof(response), 0);
    
    if (bytesRead == -1) {
        perror("Error al recibir datos del servidor");
        close(clientSocket);
        exit(EXIT_FAILURE);
    } else if (bytesRead > 0) {
        response[bytesRead] = '\0';
        if (strcmp(response, "Inicio de sesion incorrecto") == 0) {
            cout << "Autenticación fallida. Cerrando la conexión." << endl;
            close(clientSocket);
            exit(EXIT_FAILURE);
        } else {
            cout << "Autenticación exitosa." << endl;
        }
    }
}

int menu(int clientSocket){
    int opcion;
    cout << "INGRESE OPCION A REALIZAR:" << endl;
    cout << "1. Sumar" << endl;
    cout << "2. Multiplicar" << endl;
    cout << "3. Dividir" << endl;
    cout << "4. Salir" << endl;
    cout << "Eliga opcion: ";
    cin >> opcion;
    send(clientSocket, &opcion, sizeof(opcion), 0);
    return opcion;
}

void inpOutNum(int clientSocket){
    int num1,num2;
    cout << "INGRESE NUMEROS:" << endl;
    cout << "Ingrese el primer número: ";
    cin >> num1;
    cout << "Ingrese el segundo número: ";
    cin >> num2;
    string numeros = to_string(num1) + " " + to_string(num2);
    sendMessage(clientSocket, numeros);
    receiveAndDisplayMessage(clientSocket); 
}

int main() {
    string serverIP = "127.0.0.1"; // Dirección IP del servidor
    int serverPort = 12345;       // Puerto del servidor
    int clientSocket = connectToServer(serverIP, serverPort);
    login(clientSocket);
    bool again = true;
    while (again){
        int opcion = menu(clientSocket);
        if (opcion != 4){
            inpOutNum(clientSocket);
        } 
        else {
            again = false;
            }
   } 
    //close(clientSocket);

    return 0;
}
