#include <iostream>
#include <cstring>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>

#define MSG_SIZE 64

void intermedio();
mqd_t mtx, mutex; /* cola de mensajes para comunicar los procesos */

int main(int argc, char* argv[]) {
    struct mq_attr attr; /* atributos de la cola de mensajes */

    attr.mq_maxmsg = 1; /* tamaño de la cola */
    attr.mq_msgsize = MSG_SIZE; /* tamaño del mensaje */
    mutex = mq_open("/MUTEX", O_RDWR, 0777, &attr);
    if (mutex == -1) { std::cerr << "Error: " << strerror(errno) << std::endl; std::cerr << "fallo mutex" << std::endl; exit(0); }

    mtx = mq_open("/MTX", O_RDWR, 0777, &attr);
    if (mtx == -1) { std::cerr << "Error: " << strerror(errno) << std::endl; std::cerr << "fallo mtx" << std::endl; exit(0); }
    intermedio();
}

void intermedio() {
    char datos_recibidos[MSG_SIZE];
    char datos_envios_a_proceso_fin[MSG_SIZE];
    if (mq_receive(mutex, datos_recibidos, sizeof(datos_recibidos), 0) == -1) {
        std::cerr << "Receive intermedio Error: " << strerror(errno) << std::endl;
        exit(0);
    }
    std::cout << "proceso intermedio: " << datos_recibidos << std::endl;
    std::strcpy(datos_envios_a_proceso_fin, "envio desde proceso intermedio un ");
    std::strcat(datos_envios_a_proceso_fin, datos_recibidos);
    std::cout << "proceso intermedio: " << datos_envios_a_proceso_fin << std::endl;
    if (mq_send(mtx, datos_envios_a_proceso_fin, sizeof(datos_envios_a_proceso_fin), 0) == -1) {
        std::cerr << "Send intermedio Error: " << strerror(errno) << std::endl;
        exit(0);
    }
}
