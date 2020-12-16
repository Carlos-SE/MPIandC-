#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <mpi.h>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int num_filosofos = 5,
          num_procesos = 2*num_filosofos + 1,
          etiq_coger = 0,
          etiq_soltar = 1,
          etiq_sentarse = 2,
          etiq_levantarse = 3,
          camarero = 0;




template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


void funcion_filosofo(int id){
    int id_ten_izq = (id + 1) % num_procesos;
    int id_ten_der = (id - 1);
    int peticion;

    if (id_ten_izq == 0)
        id_ten_izq = 1;

    while (true){
        cout <<"Filósofo " << id << " solicita sentarse." << endl << flush;
        MPI_Send(&peticion, 1, MPI_INT, camarero, etiq_sentarse, MPI_COMM_WORLD);

        cout <<"Filósofo " << id << " solicita ten. izq." << id_ten_izq << endl << flush;
        MPI_Send(&peticion, 1, MPI_INT, id_ten_izq, etiq_coger, MPI_COMM_WORLD);

        cout <<"Filósofo " << id << " solicita ten. der." << id_ten_der << endl << flush;
        MPI_Send(&peticion, 1, MPI_INT, id_ten_der, etiq_coger, MPI_COMM_WORLD);

        cout <<"Filósofo " << id << " comienza a comer." << endl << flush;
        sleep_for(milliseconds(aleatorio<10,100>()));

        cout <<"Filósofo " << id << " suelta ten. izq." << id_ten_izq << endl << flush;
        MPI_Send(&peticion, 1, MPI_INT, id_ten_izq, etiq_soltar, MPI_COMM_WORLD);

        cout <<"Filósofo " << id << " suelta ten. der." << id_ten_der << endl << flush;
        MPI_Send(&peticion, 1, MPI_INT, id_ten_der, etiq_soltar, MPI_COMM_WORLD);

        cout << "Filósofo " << id << " se levanta." << endl << flush;
        sleep_for(milliseconds(aleatorio<10,100>()));
        
    }
}

void funcion_tenedor(int id){
    int peticion, id_filosofo;
    MPI_Status estado;

    while (true){

        MPI_Recv(&peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiq_coger, MPI_COMM_WORLD, &estado);
        id_filosofo = estado.MPI_SOURCE;

        cout << "Tenedor " << id << " ha sido cogido por filo. " << id_filosofo << endl << flush;

        MPI_Recv(&peticion, 1, MPI_INT, id_filosofo, etiq_soltar, MPI_COMM_WORLD, &estado);

        cout << "Tenedor " << id << " ha sido soltado por filo. " << id_filosofo << endl << flush;
    }
}

void funcion_camarero(){
    int peticion, etiq_aceptable, sentados = 0;
    MPI_Status estado;

    while (true){
        if (sentados < num_filosofos - 1){
            etiq_aceptable = MPI_ANY_TAG;
        }
        else{
            etiq_aceptable = etiq_levantarse;
        }

        MPI_Recv(&peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado);

        if (estado.MPI_TAG == etiq_sentarse){
            sentados++;
            cout << "El filo. " << estado.MPI_SOURCE << " se sienta. Hay " << sentados << " filósofos sentados." << endl << flush;
        }

        else if (estado.MPI_TAG == etiq_levantarse){
            sentados--;
            cout << "El filo. " << estado.MPI_SOURCE << " se levanta. Hay " << sentados << " filósofos sentados." << endl << flush;
        }

        else{
            cout << "El camarero ha recibido un mensaje erróneo" << endl << flush;
            exit(1);
        }

    }
}



int main(int argc, char *argv[]){
    int id_propio, num_procesos_actual;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);

    if (num_procesos == num_procesos_actual){
        if (id_propio == camarero){
            funcion_camarero();
        }
        else if (id_propio%2 == 0){
            funcion_filosofo(id_propio);
        }
        else{
            funcion_tenedor(id_propio);
        }
    }

    else{
        if (id_propio == 0){
            cout << "El número de procesos esperado es: " << num_procesos << endl;
            cout << "El número de procesos introducido es: " << num_procesos_actual << endl;
            cout << "(programa abortado)" << endl;
        }
    }

    MPI_Finalize();
    return 0;
}
