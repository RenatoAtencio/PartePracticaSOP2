#include <sys/types.h>
#include <unistd.h>
#include <queue>
#include <iostream>
#include <wait.h>
#include <algorithm>


using namespace std;

struct Procesos {
    int id;
    int arrivalTime;
    int duration;

    Procesos(int _id, int _arrivalTime, int _duration) : id(_id), arrivalTime(_arrivalTime), duration(_duration) {}
};

void runFIFO(queue<Procesos>& processes) {
    int time = 0;

    while (!processes.empty()) {
        Procesos currentProcess = processes.front();
        processes.pop();

        // Simula la ejecución del proceso
        usleep(currentProcess.duration * 1000000);  // Simulando en segundos

        // Imprime el tiempo de finalización y el ID del proceso
        cout << "Proceso " << currentProcess.id << ": \n\ttiempo de llegada: "<< currentProcess.arrivalTime << " \n\ttiempo de espera: "<< time-currentProcess.arrivalTime<< "\n\tduracion: "<< currentProcess.duration << "\n\tfinalizado en el tiempo: " << currentProcess.duration+time << endl;
        time += currentProcess.duration;  // Actualiza el tiempo actual
    }
}

queue<Procesos> genQueueProcesos(int num) {
    vector<Procesos> procesos;
    queue<Procesos> fifoQueue;

     srand(time(0));
     for (int i = 1; i <= num; ++i) {
        int random_arrival = rand() % 10;   
        int random_duration = rand() % 5 + 1;  
        procesos.push_back(Procesos(i, random_arrival, random_duration));
    }

    // Sort the processes based on arrival time
    sort(procesos.begin(), procesos.end(), [](const Procesos &a, const Procesos &b) {
        return a.arrivalTime < b.arrivalTime;
    });

    for (int i = 0; i < num; ++i) {
        fifoQueue.push(procesos[i]);
    }
    return fifoQueue;
}

int main() {
    // Creamos una cola de procesos (FIFO)
    int num_procesos = 5;
    // funcion para crear procesos aleatorios
    queue<Procesos> fifoQueue;
    fifoQueue = genQueueProcesos(num_procesos);

    // Insertamos algunos procesos en la cola
    /*
    fifoQueue.push(Procesos(1, 0, 4));
    fifoQueue.push(Procesos(2, 1, 3));
    fifoQueue.push(Procesos(3, 4, 5));
    fifoQueue.push(Procesos(4, 8, 5));
    */
    pid_t pid = fork();

    if (pid == -1) {
        perror("Error al crear el proceso hijo");
        return 1;
    } else if (pid == 0) {
        // Código para el proceso hijo
        cout << "Soy el proceso hijo (PID: " << getpid() << ")" << endl;
        runFIFO(fifoQueue);  // Ejecuta los procesos en orden FIFO para este proceso hijo
        return 0;  // Importante terminar el proceso hijo después de ejecutar los procesos
    }

    // Espera a que todos los procesos hijos finalicen
    for (int i = 0; i < num_procesos; ++i) {
        wait(NULL);
    }

    // Código para el proceso padre
    cout << "Soy el proceso padre (PID: " << getpid() << ")" << endl;

    return 0;
}
