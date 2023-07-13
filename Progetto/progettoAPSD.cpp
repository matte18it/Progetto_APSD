#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <fstream>
#include <iostream>
//#include <allegro.h>
#include <cmath>
#include <pthread.h>
#include <queue>
//#include "Allegro/printAllegro.h"

using namespace std;

#define NCOLS 20
#define NROWS 8
#define WIDTH NCOLS*20
#define HEIGHT NROWS*20

#define v(r,c) ((r)*(NCOLS/xPartitions+2)+(c))
#define h(r,c) ((r)*(NCOLS)+(c))
MPI_Datatype bigMtype;
MPI_Datatype columnType, rec;
MPI_Datatype sendPrint;

int xPartitions, yPartitions, nThreads, steps;
int *readM;
int *writeM;
int *bigM;
int Rank, nProc, rankUp, rankDown, rankLeft, rankRight;
int controlledCell=0;
bool ending=false;
//Gestione pthread con thread pool
pthread_mutex_t mutex;
pthread_cond_t cond ;

void transitionFunction(int x, int y){
    int cont=0;// Conto i vicini vivi
	for(int di=-1;di<2;di++)
		for(int dj=-1;dj<2;dj++)
			if ((di!=0 || dj!=0) &&readM[v((x+di+NROWS)%NROWS,(y+dj+NCOLS)%NCOLS)]==1)
				cont++;
	// Regole Gioco della Vita
    pthread_mutex_lock(&mutex);
	if (readM[v(x,y)]==1)
		if (cont==2 || cont ==3)
			writeM[v(x,y)]=1;
		else
			writeM[v(x,y)]=0;
	else
		if (cont ==3)
			writeM[v(x,y)]=1;
		else
			writeM[v(x,y)]=0;
    pthread_mutex_unlock(&mutex);
}

struct cell{
    cell(int i, int j):i(i), j(j){}
    cell(){}
    int i;
    int j;
};
queue<cell> q;
void * run(void * arg){
    while (!ending || !q.empty()){
    pthread_mutex_lock(&mutex);
    while (q.empty())
        pthread_cond_wait(&cond, &mutex);

    cell c=q.front();
    q.pop();
    pthread_mutex_unlock(&mutex);
    transitionFunction(c.i, c.j);
    
    }
    
    
    
return NULL;
}


pthread_t *threads;

//variabili allegro
//printAllegro printAl;

//BITMAP *buffer;
int nero, bianco;
#define WIDTH NCOLS*20
#define HEIGHT NROWS*20

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
    threads=new pthread_t[nThreads];
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
                else{
                    readM[v(i,j)]=bigM[h(i-1,j-1)];
                    }
            }
        }
        for(int i=0; i<yPartitions; i++){
            for(int j=0; j<xPartitions; j++){
                if(i==0 && j==0){
                    continue;
                }else{
                    
                    MPI_Send(&bigM[h(i*(NROWS/yPartitions),j*(NCOLS/xPartitions))], 1, bigMtype, dest, 0, MPI_COMM_WORLD);
                dest++;}
                
            }
        } 
        

    }else{
        MPI_Status stat;
        for(int i=0; i<NROWS/yPartitions+2; i++){
            for(int j=0; j<NCOLS/xPartitions+2; j++){
                readM[v(i,j)]=0;
            }
        }
        MPI_Recv(&readM[v(1,1)], 1, rec, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
        
        
    }
   


}

void exchBoard(){
    MPI_Request request;
    MPI_Status status;
    int c;
    MPI_Send(&readM[v(0,NCOLS/xPartitions)], 1, columnType, rankRight, 17, MPI_COMM_WORLD);
    MPI_Send(&readM[v(0,1)], 1, columnType, rankLeft, 20, MPI_COMM_WORLD);
    MPI_Recv(&readM[v(0,0)], 1, columnType, rankLeft, 17, MPI_COMM_WORLD, &status);
    MPI_Recv(&readM[v(0,NCOLS/xPartitions+1)], 1, columnType, rankRight, 20, MPI_COMM_WORLD, &status);
    

    MPI_Send(&readM[v(NROWS/yPartitions,0)], NCOLS/xPartitions+2, MPI_INT, rankDown, 12, MPI_COMM_WORLD);
	MPI_Send(&readM[v(1,0)], NCOLS/xPartitions+2, MPI_INT, rankUp, 15, MPI_COMM_WORLD);
	MPI_Recv(&readM[v(NROWS/yPartitions+1,0)], NCOLS/xPartitions+2, MPI_INT, rankDown, 15, MPI_COMM_WORLD, &status);
	MPI_Recv(&readM[v(0,0)], NCOLS/xPartitions+2, MPI_INT, rankUp, 12, MPI_COMM_WORLD, &status);


}
void print(){
    //I processi mandano la loro porzione al processo 0 per stampare
    if(Rank!=0){
        MPI_Send(&readM[v(1,1)], 1, sendPrint, 0, 29, MPI_COMM_WORLD);
    }else{
        int dest=1;
        MPI_Status stat;
        for(int i=0; i<yPartitions; i++){
            for(int j=0; j<xPartitions; j++){
                if(i==0 && j==0){
                    continue;
                }else{
                    
                    MPI_Recv(&bigM[h(i*(NROWS/yPartitions),j*(NCOLS/xPartitions))], 1, bigMtype, dest, 29, MPI_COMM_WORLD, &stat);
                dest++;}
                
            }
        } }
//Stampa di bigM (adesso senza allegro)
    if(Rank==0){
    for(int i=0; i<NROWS; i++){
        for(int j=0; j<NCOLS; j++){
            printf("%d ", bigM[h(i, j)]);
        }
        printf("\n");
    }
    printf("-----------------------------------------------\n");}
}

inline void transFunc(){   
	for(int i=1;i<NROWS/yPartitions+1;i++){
		for(int j=1;j<NCOLS/xPartitions+1;j++){
            cell c(i,j);
            q.push(c);
            pthread_cond_broadcast(&cond);
            
			}}}
void swap(){
   
    while (!q.empty()) {
        sleep(1);
    }
    
    int * p=readM;
    readM=writeM;
    writeM=p;}

void initPthread(){
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    for(int i=0; i<nThreads; i++){
        pthread_create(&threads[i], NULL, &run, NULL);
    }
}
int main(int argc, char *argv[]) {
    MPI_Init( &argc, &argv );    
    MPI_Comm_rank( MPI_COMM_WORLD, &Rank );    
    MPI_Comm_size( MPI_COMM_WORLD, &nProc);
if(Rank==0)
        bigM=new int[NROWS*NCOLS];
    init();



    //inizializzo allegro
    //printAl.initAllegro(Rank, WIDTH, HEIGHT);

    MPI_Type_vector(NROWS/yPartitions, NCOLS/xPartitions, (NCOLS/xPartitions)*xPartitions, MPI_INT, &bigMtype);
    MPI_Type_commit(&bigMtype);  
    MPI_Type_vector(NROWS/yPartitions, NCOLS/xPartitions, (NCOLS/xPartitions)+2, MPI_INT, &rec);
    MPI_Type_commit(&rec);  
    MPI_Type_vector(NROWS/yPartitions+2, 1, NCOLS/xPartitions+2, MPI_INT, &columnType);    
    MPI_Type_commit(&columnType);
    MPI_Type_vector(NROWS/yPartitions, NCOLS/xPartitions, (NCOLS/xPartitions)+2, MPI_INT, &sendPrint);
    MPI_Type_commit(&sendPrint);  
    
    

    readM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    writeM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    

    rankUp = (Rank - xPartitions);
    if(rankUp<0)
        rankUp = (rankUp+nProc);
    rankDown = (Rank + xPartitions);
    if(rankDown>=nProc)
        rankDown = (rankDown%nProc);
    
    float colonna=Rank/xPartitions;
    float colonnameno=(Rank-1)/xPartitions;
    if(Rank-1<0 || colonnameno<colonna)
        rankLeft = (Rank+xPartitions-1);
    else
        rankLeft = (Rank-1);
    float colonnapiu=(Rank+1)/xPartitions;
    if(Rank+1>=nProc || colonnapiu>colonna)
        rankRight = (Rank-xPartitions+1);
    else
        rankRight = (Rank+1);
       


    initAutoma();
    initPthread();

    
    //disegno con allegro
    //printAl.drawWithAllegro(NCOLS, xPartitions, yPartitions, NROWS, WIDTH, HEIGHT, readM, Rank);

    for(int i=0; i<steps; i++){
        exchBoard();
       
        print();
        transFunc();
        swap();
    }
    printf("ciao");
    ending=true;
    pthread_cond_broadcast(&cond);
    for(int i=0; i<nThreads; i++){
        pthread_join(threads[i], NULL);
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    

    delete[] readM;
    delete[] writeM;
    if(Rank==0)
        delete[] bigM;  
    MPI_Type_free(&bigMtype);
    MPI_Type_free(&rec);
    MPI_Type_free(&columnType);
    MPI_Type_free(&sendPrint);

    MPI_Finalize();  
	return 0;
}
//END_OF_MAIN();