#include <iostream>
#include <cstring>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>

#define MSG_SIZE 64

void inicio();
mqd_t mtx, mutex; /* cola de mensajes para comunicar los procesos */

int main(int argc, char* argv[]) {
    struct mq_attr attr; /* atributos de la cola de mensajes */

    attr.mq_maxmsg = 1; /* tamaño de la cola */
    attr.mq_msgsize = MSG_SIZE; /* tamaño del mensaje */
    mutex = mq_open("/MUTEX", O_CREAT | O_RDWR, 0777, &attr);
    if (mutex == -1) { std::cerr << "Error: " << strerror(errno) << std::endl; std::cerr << "fallo mutex" << std::endl; exit(0); }

    mtx = mq_open("/MTX", O_CREAT | O_RDWR, 0777, &attr);
    if (mtx == -1) { std::cerr << "Error: " << strerror(errno) << std::endl; std::cerr << "fallo mtx" << std::endl; exit(0); }
    inicio();
}

void inicio() {
    char datos_envios[MSG_SIZE];
    std::string mensaje = "token";
    std::strcpy(datos_envios, mensaje.c_str());
    std::cout << "proceso inicio: " << datos_envios << std::endl;
    if (mq_send(mutex, datos_envios, sizeof(datos_envios), 0) == -1) {
        std::cerr << "Send inicio Error: " << strerror(errno) << std::endl;
        exit(0);
    }
}
