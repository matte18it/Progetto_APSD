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
int Rank, nProc;
void loadConfiguration(){
    ifstream configurazione("Configuration.txt");
    if(configurazione.is_open()){
        configurazione >> xPartitions >> yPartitions >> nThreads >> steps;
        configurazione.close();
    }else{
        printf("Errore nell'apertura del file di configurazione");
        exit(1);
    }
}
void loadBigM(){
     int *bigM = new int[NCOLS*NROWS];
        ifstream Input("Input.txt");
        if(Input.is_open()){
            char c;
            int i=0;
            while (Input.get(c)){
                if(c=='\n' ||  c=='\r' || c==' ')
                    continue;
                bigM[i]=c-'0';
                i++;
        } 
            Input.close();
        }else{
            printf("Errore nell'apertura del file di Input");
            exit(1);
    }
}

void init(){
    loadConfiguration();
    if(Rank==0)
        loadBigM();
}

int main(int argc, char *argv[]) {    
    MPI_Init( &argc, &argv );    
    MPI_Comm_rank( MPI_COMM_WORLD, &Rank );    
    MPI_Comm_size( MPI_COMM_WORLD, &nProc);  

    init();
    readM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    writeM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    
    
   
    
    
    




    MPI_Finalize();  

	

	return 0;

}