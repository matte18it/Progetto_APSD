#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <fstream>
#include <iostream>
#include <allegro.h>
using namespace std;

#define NCOLS 20
#define NROWS 8

#define v(r,c) ((r)*(NCOLS)+(c))
MPI_Datatype columnType;

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

int main(int argc, char *argv[]) {
    MPI_Init( &argc, &argv );    
    MPI_Comm_rank( MPI_COMM_WORLD, &Rank );    
    MPI_Comm_size( MPI_COMM_WORLD, &nProc);

    init();

    //inizializzo allegro
    initAllegro();

    MPI_Type_vector(NROWS/yPartitions, NCOLS/xPartitions, (NCOLS/xPartitions)*2, MPI_INT, &columnType);
    MPI_Type_commit(&columnType);  

    readM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    writeM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];

    initAutoma();

    if(Rank == 1){
        printf("RANK 1\n");
        for(int i=1; i<NROWS/yPartitions+1; i++){
            for(int j=1; j<NCOLS/xPartitions+1; j++){
                printf("%d ", readM[v(i,j)]);
            }
            printf("\n");
        }
    }

    drawWithAllegro();

    MPI_Finalize();  
	return 0;

}
END_OF_MAIN();