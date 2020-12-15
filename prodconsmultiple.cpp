#include <mpi.h>
#include <iostream>
#include <thread>
#include <random>
#include <chrono>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int id_buffer = 4,
          num_consumidores = 5,
          num_productores = 4,
          num_procesos = 10,
          num_items = 20,
          tam_vector = 10,
          tag_prod = 0,
          tag_cons = 1;

template< int min, int max > int aleatorio()
{
    static default_random_engine generador( (random_device())() );
    static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
    return distribucion_uniforme( generador );
}

int producir()
{
    static int contador = 0 ;
    sleep_for( milliseconds( aleatorio<10,100>()) );
    contador++ ;
    cout << "Productor ha producido valor " << contador << endl << flush;
    return contador ;
}

void funcion_productor()
{
   for ( unsigned int i= 0 ; i < num_items ; i++ )
   {
        // producir valor
        int valor_prod = producir();
        // enviar valor
        cout << "Productor va a enviar valor " << valor_prod << endl << flush;
        MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, tag_prod, MPI_COMM_WORLD );
   }
}

///////////////////////////////////////////////////////////////////////////////
void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              primera_ocupada     = 0, // índice de primera celda ocupada
              num_celdas_ocupadas = 0, // número de celdas ocupadas
              tag_emisor_aceptable ;    // identificador de emisor aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido

   for( unsigned int i=0 ; i < num_items*2 ; i++ )
   {
      // 1. determinar si puede enviar solo prod., solo cons, o todos

      if ( num_celdas_ocupadas == 0 )               // si buffer vacío
         tag_emisor_aceptable = tag_prod;       // $~~~$ solo prod.
      else if ( num_celdas_ocupadas == tam_vector ) // si buffer lleno
         tag_emisor_aceptable = tag_cons;      // $~~~$ solo cons.
      else                                          // si no vacío ni lleno
         tag_emisor_aceptable = MPI_ANY_TAG;     // $~~~$ cualquiera

      // 2. recibir un mensaje del emisor o emisores aceptables

      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, tag_emisor_aceptable, MPI_COMM_WORLD, &estado );

      // 3. procesar el mensaje recibido

      switch( estado.MPI_TAG) // leer emisor del mensaje en metadatos
      {
         case tag_prod: // si ha sido el productor: insertar en buffer
            buffer[primera_libre] = valor ;
            primera_libre = (primera_libre+1) % tam_vector ;
            num_celdas_ocupadas++ ;
            cout << "Buffer ha recibido valor " << valor << endl ;
            break;

         case tag_cons: // si ha sido el consumidor: extraer y enviarle
            int id_cons = estado.MPI_SOURCE;
            valor = buffer[primera_ocupada] ;
            primera_ocupada = (primera_ocupada+1) % tam_vector ;
            num_celdas_ocupadas-- ;
            cout << "Buffer va a enviar valor " << valor << endl ;
            MPI_Ssend( &valor, 1, MPI_INT, id_cons,0, MPI_COMM_WORLD);
            break;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

void consumir( int valor_cons )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor ha consumido valor " << valor_cons << endl << flush ;
}
void funcion_consumidor()
{
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < num_items; i++ )
   {
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, tag_cons, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, 0, MPI_COMM_WORLD,&estado );
      cout << "Consumidor ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec );
   }
}




int main(int argc, char *argv[]){

    int id_actual, num_proceso_actual;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &id_actual );
    MPI_Comm_size( MPI_COMM_WORLD, &num_proceso_actual );

    if (num_proceso_actual == num_procesos){
        if (id_actual < num_productores){
            funcion_productor();
        }
        else if (id_actual == num_productores){
            funcion_buffer();
        }
        else{
            funcion_consumidor();
        }
    }
    else{
        if (id_actual == 0){
            cout << "Número de procesos introducidos: " << num_proceso_actual << endl;
            cout << "Número de procesos esperado: " << num_procesos << endl;
            cout << "(programa abortado)" << endl;

        }
    }
}