#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <fstream>
#include <iostream>
#include <allegro.h>
#include <cmath>
using namespace std;

#define NCOLS 20
#define NROWS 8

#define v(r,c) ((r)*(NCOLS)+(c))
MPI_Datatype bigMtype, columnType;

int xPartitions, yPartitions, nThreads, steps;
int *readM;
int *writeM;
int *bigM;
int Rank, nProc, rankUp, rankDown, rankLeft, rankRight;

//variabili allegro
BITMAP *buffer;
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
                    MPI_Send(&bigM[v(i*(NROWS/yPartitions),j*(NCOLS/xPartitions))], 1, bigMtype, dest, 0, MPI_COMM_WORLD);
                dest++;
                
            }
        } 
        

    }else{
        for(int i=0; i<NROWS/yPartitions+2; i++){
            for(int j=0; j<NCOLS/xPartitions+2; j++){
                readM[v(i,j)]=0;
            }
        }
        MPI_Recv(&readM[v(1,1)], 1, bigMtype, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
    }
   


}

void initAllegro() {
	allegro_init();
    install_keyboard();
	set_color_depth(24);
	buffer = create_bitmap(WIDTH, HEIGHT);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIDTH, HEIGHT, 0, 0);

    char windowTitle[50];
    sprintf(windowTitle, "Process %d", Rank);
    set_window_title(windowTitle);

	nero = makecol(0, 0, 0);
	bianco = makecol(255, 255, 255);
}

void drawWithAllegro() {
    int const CELL_SIZE = WIDTH / (NCOLS / xPartitions);

	for (int i = 1; i < NROWS/yPartitions+1; i++)
		for (int j = 1; j < NCOLS/xPartitions+1; j++){
            int x = (i-1) * CELL_SIZE;
            int y = (j-1) * CELL_SIZE;
            //printf("X1:%d   X2:%d   Y1:%d   Y2:%d, i:%d    j:%d   readM:%d   rank:%d\n", x, x+CELL_SIZE, y, y+CELL_SIZE, i, j, readM[v(i, j)], Rank);
            switch (readM[v(i, j)]) {
			case 0:
				rectfill(buffer, y, x, y + CELL_SIZE, x + CELL_SIZE, nero);
				break;
			case 1:
				rectfill(buffer, y, x, y + CELL_SIZE, x + CELL_SIZE, bianco);
				break;
			}
        }
	blit(buffer, screen, 0, 0, 0, 0, WIDTH, HEIGHT);
    readkey();
}

void exchBoard(){
    MPI_Request request;
    MPI_Status status;
    MPI_Isend(&readM[v(0,NCOLS/xPartitions)], 1, columnType, rankRight, 17, MPI_COMM_WORLD, &request);
    MPI_Isend(&readM[v(0,1)], 1, columnType, rankLeft, 20, MPI_COMM_WORLD, &request);
    MPI_Recv(&readM[v(0,0)], 1, columnType, rankLeft, 17, MPI_COMM_WORLD, &status);
    MPI_Recv(&readM[v(0,NCOLS/xPartitions+1)], 1, columnType, rankRight, 20, MPI_COMM_WORLD, &status);

    MPI_Isend(&readM[v(NROWS/yPartitions,0)], NCOLS/xPartitions, MPI_INT, rankDown, 12, MPI_COMM_WORLD, &request);
	MPI_Isend(&readM[v(1,0)], NCOLS/xPartitions, MPI_INT, rankUp, 15, MPI_COMM_WORLD, &request);
	MPI_Recv(&readM[v(NROWS/yPartitions+1,0)], NCOLS/xPartitions, MPI_INT, rankDown, 15, MPI_COMM_WORLD, &status);
	MPI_Recv(&readM[v(0,0)], NCOLS/xPartitions, MPI_INT, rankUp, 12, MPI_COMM_WORLD, &status);


}
int main(int argc, char *argv[]) {
    MPI_Init( &argc, &argv );    
    MPI_Comm_rank( MPI_COMM_WORLD, &Rank );    
    MPI_Comm_size( MPI_COMM_WORLD, &nProc);

    init();

    //inizializzo allegro
    initAllegro();

    MPI_Type_vector(NROWS/yPartitions, NCOLS/xPartitions, (NCOLS/xPartitions)*2, MPI_INT, &bigMtype);
    MPI_Type_commit(&bigMtype);  
    MPI_Type_vector(NROWS/yPartitions+2, 1, NCOLS/xPartitions+2, MPI_INT, &columnType);
    MPI_Type_commit(&columnType);

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
//drawWithAllegro();
    for(int i=0; i<steps; i++){
        exchBoard();
    }

    

    
    delete[] readM;
    delete[] writeM;
    if(Rank==0)
        delete[] bigM;  
    MPI_Finalize();  
	return 0;

}
END_OF_MAIN();