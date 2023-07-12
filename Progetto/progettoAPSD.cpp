#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <fstream>
#include <iostream>
using namespace std;
#define NCOLS 20
#define NROWS 20

int xPartitions, yPartitions, nThreads, steps;


int main(int argc, char *argv[]) {
     int rank, size;    
    MPI_Init( &argc, &argv );    
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );    
    MPI_Comm_size( MPI_COMM_WORLD, &size );   
    ifstream configurazione("Configuration.txt");
    if(configurazione.is_open()){
        configurazione >> xPartitions >> yPartitions >> nThreads >> steps;
        configurazione.close();
    }else{
        printf("Errore nell'apertura del file di configurazione");
        exit(1);
    }
    ifstream Input("Input.txt");
    if(configurazione.is_open()){
        configurazione >> xPartitions >> yPartitions >> nThreads >> steps;
        configurazione.close();
    }else{
        printf("Errore nell'apertura del file di configurazione");
        exit(1);
    }
    
    




    MPI_Finalize();  

	

	return 0;

}