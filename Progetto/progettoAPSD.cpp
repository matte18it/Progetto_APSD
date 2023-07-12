#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <fstream>
#include <iostream>

#include <allegro5/allegro.h>
#include "PrintAllegro.h"
#include "allegro/Init.h"
#include "allegro/Frame.h"
#include "allegro/Panel.h"
using namespace allegro;

using namespace std;
#define NCOLS 20
#define NROWS 8
Panel *p;

#define v(r,c) ((r)*(NCOLS)+(c))
MPI_Datatype columnType;

int xPartitions, yPartitions, nThreads, steps;
int *readM;
int *writeM;
int *bigM;
int Rank, nProc, rankUp, rankDown, rankLeft, rankRight;
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
        bigM = new int[NCOLS*NROWS];
        ifstream Input("Input.txt");
        if(Input.is_open()){
            char c;
            int i=0;
            while (Input.get(c)){
                if(c!='0' && c!='1')
                    continue;
                bigM[i]=c - '0';
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
void initAutoma(){
    if(Rank==0){
        int dest=1;
        for(int i=0; i<NROWS/yPartitions+2; i++){
            for(int j=0; j<NCOLS/xPartitions+2; j++){
                if(i==0 || i==NROWS/yPartitions+1 || j==0 || j==NCOLS/xPartitions+1)
                    readM[v(i,j)]=0;
                else
                    readM[v(i,j)]=bigM[v(i-1,j-1)];
            }
        }
        for(int i=0; i<yPartitions; i++){
            for(int j=0; j<xPartitions; j++){
                if(i==0 && j==0){
                    continue;
                }else
                    MPI_Send(&bigM[v(i*(NROWS/yPartitions),j*(NCOLS/xPartitions))], 1, columnType, dest, 0, MPI_COMM_WORLD);
                dest++;
                
            }
        } 
        

    }else{
        for(int i=0; i<NROWS/yPartitions+2; i++){
            for(int j=0; j<NCOLS/xPartitions+2; j++){
                readM[v(i,j)]=0;
            }
        }
        MPI_Recv(&readM[v(1,1)], 1, columnType, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
    }
}

int main(int argc, char *argv[]) {   

    MPI_Init( &argc, &argv );    
    MPI_Comm_rank( MPI_COMM_WORLD, &Rank );    
    MPI_Comm_size( MPI_COMM_WORLD, &nProc);

    init();

    MPI_Type_vector(NROWS/yPartitions, NCOLS/xPartitions, (NCOLS/xPartitions)*2, MPI_INT, &columnType);
    MPI_Type_commit(&columnType);  

    readM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    writeM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];

    initAutoma();
    if(Rank == 0){
        //il processo con rank 0 crea il pannello di disegno

        //codice di test
        for(int i=0; i<NROWS; i++)
            for(int j=0; j<NCOLS; j++)
                readM[v(i,j)]=0;
        readM[v(5, 4)] = 1;
        readM[v(4, 4)] = 1;
        readM[v(3, 4)] = 1;
        readM[v(2, 3)] = 1;
        readM[v(7, 19)] = 1;
        //fine codice di test
        
        Init::init();
        Frame f(NCOLS*50, NROWS*50, "ProgettoAPSD"); 
        p = new PrintAllegro(readM, NROWS, NCOLS);
        f.add(p);
        p->repaint();
        Init::sleep(2);
    }
    if(Rank==1){
        for(int i=1; i<NROWS/yPartitions+1; i++){
            for(int j=1; j<NCOLS/xPartitions+1; j++){
                printf("%d ", readM[v(i,j)]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();  
	return 0;

}