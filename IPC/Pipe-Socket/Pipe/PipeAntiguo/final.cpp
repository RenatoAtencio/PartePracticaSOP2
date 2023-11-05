#include <iostream>
#include <cstring>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>

#define MSG_SIZE 64

void final();
mqd_t mtx; /* cola de mensajes para comunicar los procesos */

int main(int argc, char* argv[]) {
    struct mq_attr attr; /* atributos de la cola de mensajes */

    attr.mq_maxmsg = 1; /* tamaño de la cola */
    attr.mq_msgsize = MSG_SIZE; /* tamaño del mensaje */

    mtx = mq_open("/MTX", O_RDWR, 0777, &attr);
    if (mtx == -1) { std::cerr << "Error: " << strerror(errno) << std::endl; std::cerr << "fallo mtx" << std::endl; exit(0); }
    final();
    mq_close(mtx);
    mq_unlink("/MTX");
}

void final() {
    char datos_recibidos_fin[MSG_SIZE];
    if (mq_receive(mtx, datos_recibidos_fin, sizeof(datos_recibidos_fin), 0) == -1) {
        std::cerr << "Receive final Error: " << strerror(errno) << std::endl;
        exit(0);
    }
    std::cout << "proceso final: " << datos_recibidos_fin << std::endl;
}
