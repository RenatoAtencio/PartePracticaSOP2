#include <iostream>
#include <thread>
#include <semaphore.h>
#include <vector>

using namespace std;

const int NUM_ASIENTOS = 4;

sem_t semAsientos;

void cliente(int id) {
    srand(time(0));
    int time = 3+rand() % (10-3) ;
    //int time = 5;
    cout << "Cliente " << id << " llega a la peluquería, se demorará: "<<time<<" segundos."<< endl;
    sem_wait(&semAsientos);

    // Simular el tiempo que el cliente está siendo atendido
    
    cout << "Cliente " << id << " se sienta en el asiento " << (id % NUM_ASIENTOS) + 1<< "." << endl;
    this_thread::sleep_for(chrono::seconds(time));
    
    // Liberar el asiento
    sem_post(&semAsientos);
    cout << "Cliente " << id << " ha terminado y se va de la peluquería, se demoró: "<<time<<" segundos." << endl;
}

int main() {
    // Inicializar el semáforo con el número de asientos disponibles
    sem_init(&semAsientos, 0, NUM_ASIENTOS);

    // Crear un vector de hilos para representar a los clientes
    vector<thread> clientes;

    // Simular la llegada de 10 clientes
    for (int i = 1; i <= 10; ++i) {
        clientes.emplace_back(cliente, i);
        this_thread::sleep_for(chrono::seconds(1));
    }

    // Esperar a que todos los hilos de clientes terminen
    for (auto& clienteThread : clientes) {
        clienteThread.join();
    }

    // Destruir el semáforo
    sem_destroy(&semAsientos);

    return 0;
}
