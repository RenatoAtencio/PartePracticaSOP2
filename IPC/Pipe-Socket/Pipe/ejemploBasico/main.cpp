#include <unistd.h>
#include <iostream>

int main() {
    int fd1[2]; // Usado para enviar el número del padre al hijo
    int fd2[2]; // Usado para enviar el cuadrado del número del hijo al padre
    pipe(fd1);
    pipe(fd2);
    pid_t pid = fork();

    if (pid > 0) { // Proceso padre
        close(fd1[0]); // Cierra el extremo de lectura
        close(fd2[1]); // Cierra el extremo de escritura

        int num = 5;
        write(fd1[1], &num, sizeof(num)); // Escribe en el extremo de escritura
        close(fd1[1]); // Cierra el extremo de escritura

        int square;
        read(fd2[0], &square, sizeof(square)); // Lee del extremo de lectura
        std::cout << "El cuadrado del número es: " << square << std::endl;
        close(fd2[0]); // Cierra el extremo de lectura
    } else if (pid == 0) { // Proceso hijo
        close(fd1[1]); // Cierra el extremo de escritura
        close(fd2[0]); // Cierra el extremo de lectura

        int num;
        read(fd1[0], &num, sizeof(num)); // Lee del extremo de lectura
        std::cout << num << std::endl;
        close(fd1[0]); // Cierra el extremo de lectura

        int square = num * num;
        write(fd2[1], &square, sizeof(square)); // Escribe en el extremo de escritura
        close(fd2[1]); // Cierra el extremo de escritura
    }

    return 0;
}
