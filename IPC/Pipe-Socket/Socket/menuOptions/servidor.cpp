#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

void operacion(int option, int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead;
    int num1, num2, result;
    string message;
    // Recibir los dos números del cliente
    bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    //cout << "Numeros recibidos: " << buffer << endl;
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        istringstream iss(buffer);
        iss >> num1 >> num2;
    } else {
        perror("error");
        return;
    }
    cout << "Numeros recibidos: " << num1 << " " << num2 << endl;
    
    switch (option) {
        case 1: // Suma
            result = num1 + num2;
            message = "El resultado de la suma es: " + to_string(result);
            break;
        case 2: // Multiplicación
            result = num1 * num2;
            message = "El resultado de la multiplicación es: " + to_string(result);
            break;
        case 3: // División
            if (num2 != 0) {
                result = num1 / num2;
                message = "El resultado de la división es: " + to_string(result);
            } else {
                message = "Error: División por cero";
            }
            break;
            
        default:
            message = "Opción no válida";
            break;
    }

    // Enviar el resultado al cliente
    send(clientSocket, message.c_str(), message.length(), 0);
}


int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Leer las credenciales del archivo
    ifstream file("credenciales.txt");
    string line;
    map<string, string> credentials;
    while (getline(file, line)) {
        istringstream iss(line);
        string username, password;
        if (!(iss >> username >> password)) { break; }
        credentials[username] = password;
    }

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
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
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

    while (true) {
        // Aceptar la conexión entrante
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Error al aceptar la conexión");
            continue; // Continuar esperando conexiones
        }

        cout << "Cliente conectado" << endl;

        // Leer y verificar las credenciales del cliente
        char buffer[1024];
        ssize_t bytesRead, optionRead;
        
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead == -1) {
            perror("Error al recibir datos");
            close(clientSocket); // Cierra la conexión con el cliente que tiene un error
            continue; // Continuar esperando conexiones
        } else if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            istringstream iss(buffer);
            string username, password;
            iss >> username >> password;
            cout << "Usuario: "<< username << " - " << "Password: " << "******" << endl; //password <<
            if (credentials.count(username) > 0 && credentials[username] == password) {
                send(clientSocket, "Inicio de sesion correcto", 30, 0);
                bool success = true;
                while (success){
                    int option;
                    // Recibe la opción como un entero desde el cliente
                    ssize_t optionRead = recv(clientSocket, &option, sizeof(option), 0);
                    cout << "Operacion recibida: " << option << endl;

                    if (option == 4){
                        close(clientSocket);
                        cout << "Cliente desconectado" << endl;
                        success = false;
                    }else{
                        operacion(option,clientSocket);
                    }
                }
            } else {
                cout << "Inicio de sesion incorrecto" << endl;
                cout << "Cliente desconectado" << endl;
                send(clientSocket, "Inicio de sesion incorrecto", 30, 0);
                close(clientSocket); // Cierra la conexión con el cliente que tiene credenciales incorrectas
            }
        }
    }

    return 0;
}
