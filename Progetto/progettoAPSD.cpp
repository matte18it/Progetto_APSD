#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <fstream>
#include <iostream>
using namespace std;
#define NCOLS 20
#define NROWS 8

int xPartitions, yPartitions, nThreads, steps;
int *readM;
int *writeM;
char *temp;


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
    //readM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    //writeM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    readM = new int[NCOLS*NROWS];
    ifstream Input("Input.txt");
    if(Input.is_open()){
        char c;
        int i=0;
        while (Input.get(c)){
            if(c=='\n' ||  c=='\r')
                continue;
            readM[i]=c-'0';
            i++;
            
        } 
        Input.close();
    }else{
        printf("Errore nell'apertura del file di configurazione");
        exit(1);
    }
    for (int i = 0; i < NROWS; i++)
    {
        for (int j = 0; j < NCOLS; j++)
        {
            printf("%d",readM[i*NCOLS+j]);
        }
        printf("\n");
    }
    
   
    
    
    




    MPI_Finalize();  

	

	return 0;

}