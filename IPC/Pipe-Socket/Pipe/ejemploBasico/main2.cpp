#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

int main() {
    // Definir el número de procesos hijos que se crearán
    const int num_hijos = 3;
    
    // Crear un vector para almacenar los descriptores de las tuberías
    std::vector<int> pipes(num_hijos * 2);

    // Crear las tuberías para la comunicación entre el proceso padre e hijos
    for (int i = 0; i < num_hijos; i++) {
        int pipe_fd[2];
        
        // Crear una tubería y verificar si se creó correctamente
        if (pipe(pipe_fd) == -1) {
            perror("Error al crear la tubería");
            exit(EXIT_FAILURE);
        }
        
        // Almacenar los descriptores de lectura y escritura en el vector de tuberías
        pipes[i * 2] = pipe_fd[0];     // Descriptor de lectura
        pipes[i * 2 + 1] = pipe_fd[1]; // Descriptor de escritura
        
    }

    // Crear procesos hijo
    for (int i = 0; i < num_hijos; i++) {
        pid_t pid = fork(); // Crear un nuevo proceso
        
        // Verificar si se creó el proceso hijo correctamente
        if (pid == -1) {
            perror("Error al crear el proceso hijo");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo
            close(pipes[i * 2]); // Cerrar el descriptor de lectura no utilizado
            
            // Crear un mensaje que el proceso hijo enviará al padre
            std::string mensaje = "Hola desde el hijo " + std::to_string(i);
            
            // Escribir el mensaje en el descriptor de escritura de la tubería
            write(pipes[i * 2 + 1], mensaje.c_str(), mensaje.size());
            
            close(pipes[i * 2 + 1]); // Cerrar el descriptor de escritura
            exit(EXIT_SUCCESS);     // Salir del proceso hijo con éxito
        }
    }

    // Leer los mensajes enviados por los procesos hijo
    for (int i = 0; i < num_hijos; i++) {
        close(pipes[i * 2 + 1]); // Cerrar el descriptor de escritura no utilizado
        char buffer[256];
        
        // Leer el mensaje desde el descriptor de lectura de la tubería
        int bytes_leidos = read(pipes[i * 2], buffer, sizeof(buffer));
        
        // Verificar errores al leer desde la tubería
        if (bytes_leidos == -1) {
            perror("Error al leer desde la tubería");
            exit(EXIT_FAILURE);
        }
        
        buffer[bytes_leidos] = '\0'; // Agregar un carácter nulo al final del mensaje
        std::cout << "Mensaje del hijo " << i << ": " << buffer << std::endl;
        close(pipes[i * 2]); // Cerrar el descriptor de lectura
    }

    // Esperar a que todos los procesos hijo terminen
    for (int i = 0; i < num_hijos; i++) {
        wait(NULL);
    }

    return 0;
}