#include <mpi.h>
#include <fstream>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <thread>       //per lo sleep
#include <allegro.h>    //libreria per allegro

//colonne e righe della matrice
#define NCOLS 20
#define NROWS 8

#define WIDTH 800
#define HEIGHT 320

//macro per accedere alla matrice locale
#define v(r,c) ((r)*(NCOLS/xPartitions+2)+(c)) 

//macro per accedere alla matrice globale
#define h(r,c) ((r)*(NCOLS)+(c)) 

//definizione dei Datatype
MPI_Datatype bigMtype, columnType, rec;

//dati da leggere dal file di configurazione
int xPartitions, yPartitions, nThreads, steps;
//matrici locali
int *readM;
int *writeM;
//matrice globale
int *bigM;
//rank e numero di processi
int Rank, nProc, rankUp, rankDown, rankLeft, rankRight;

//variabile per la gestione della fine dei pthread
bool ending=false;

//variabili allegro
BITMAP *buffer;         //creazione della bitmap, finestra in cui vado a disegnare   
int nero, bianco;       //variabili per contenere i colori, nero per lo sfondo e bianco per disegnare

//Gestione pthread con thread pool
struct cell{  // Struttura per la coda di celle da calcolare
    cell(int i, int j):i(i), j(j){}
    cell(){}
    int i;
    int j;
};
std::queue<cell> q;

pthread_mutex_t mutex, mutexWait;
pthread_cond_t cond, condWait ;
pthread_t *threads, threadWait;

//thread function
void * run(void * arg);
void * runWait(void * arg);
//funzione di transizione
void transitionFunction(int x, int y);

void loadConfiguration();
void loadBigM();
void init();
void initAutoma();
void exchBoard();
void print(int step);
void transFunc();
void swap();

//funzioni allegro
void initAllegro();         //funzione per inizializzare allegro
void drawWithAllegro(int step);     //funzione per disegnare con allegro

int main(int argc, char *argv[]) {
    MPI_Init( &argc, &argv );    
    MPI_Comm_rank( MPI_COMM_WORLD, &Rank );    
    MPI_Comm_size( MPI_COMM_WORLD, &nProc);
     
    init(); //caricamento parametri e matrici
    if(xPartitions*yPartitions!=nProc){
        if(Rank==0)
            printf("Errore: numero di processi diverso da partizioni \n");
        MPI_Finalize();
        return 0;
    }
    //inizializzazione strutture dati con i dati presi da file
    readM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];
    writeM = new int[(NROWS/yPartitions+2)*(NCOLS/xPartitions+2)];

    threads=new pthread_t[nThreads];

    //inizializzo allegro sul rank 0
    if(Rank == 0)
        initAllegro();

    //definizione dei Datatype
    //salvataggio dei dati da matrice locale a globale
    MPI_Type_vector(NROWS/yPartitions, NCOLS/xPartitions, (NCOLS/xPartitions)*xPartitions, MPI_INT, &bigMtype);
    MPI_Type_commit(&bigMtype);  
    //salvataggio dei dati da matrice globale a locale
    MPI_Type_vector(NROWS/yPartitions, NCOLS/xPartitions, (NCOLS/xPartitions)+2, MPI_INT, &rec);
    MPI_Type_commit(&rec);  
    //invio e ricezione di una colonna in matrici locali
    MPI_Type_vector(NROWS/yPartitions+2, 1, NCOLS/xPartitions+2, MPI_INT, &columnType);    
    MPI_Type_commit(&columnType);
   
    //calcolo dei rank vicini
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
    
    initAutoma(); //Divisione della matrice globale in matrici locali

    //inizializzazione pthread
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutexWait, NULL);
    pthread_cond_init(&condWait, NULL);
    for(int i=0; i<nThreads; i++){
        pthread_create(&threads[i], NULL, &run, NULL);
    }
    for(int i=0; i<steps; i++){ //Main loop
        exchBoard();  
        print(i);  
        transFunc(); 
        swap();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
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

    MPI_Finalize();  
	return 0;
    END_OF_MAIN();
}

void init(){ //Carcamento parametri da file di configurazione
    loadConfiguration();
    if(Rank==0)
        loadBigM(); //Rank 0 legge la matrice globale
}
void loadConfiguration(){ //Carcamento parametri da file di configurazione
    std::ifstream configurazione("Configuration.txt");
    if(configurazione.is_open()){
        configurazione >> xPartitions >> yPartitions >> nThreads >> steps;
        configurazione.close();
    }else{
        printf("Error opening configuration file!");
        exit(1);
    }
}
void loadBigM(){ //Carcamento matrice globale da file
        bigM = new int[NROWS*NCOLS];    
        std::ifstream Input("Input.txt");
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
            printf("Error opening Input file!");
            exit(1);
    } 
}
void initAutoma(){
    if(Rank==0){  //Rank 0 prende la prima parte della matrice globale
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
        for(int i=0; i<yPartitions; i++){  //invia le altre porzioni
            for(int j=0; j<xPartitions; j++){
                if(i==0 && j==0){
                    continue;
                }else{
                    MPI_Send(&bigM[h(i*(NROWS/yPartitions),j*(NCOLS/xPartitions))], 1, bigMtype, dest, 0, MPI_COMM_WORLD);
                dest++;}
             }
        } 
    }else{  //Gli altri rank ricevono la porzione di matrice globale
        MPI_Status stat;
        for(int i=0; i<NROWS/yPartitions+2; i++){
            for(int j=0; j<NCOLS/xPartitions+2; j++){
                readM[v(i,j)]=0;
            }
        }
        MPI_Recv(&readM[v(1,1)], 1, rec, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
    }
}
void * run(void * arg){ //thread function, ogni thread prende una cella dalla coda e la elabora
    while( !ending){
        if(q.empty())
            pthread_cond_signal(&condWait);
        pthread_mutex_lock(&mutex);
    while (q.empty() && !ending)
        pthread_cond_wait(&cond, &mutex);
    if(!q.empty()){
        cell c=q.front();
        q.pop();
        pthread_mutex_unlock(&mutex);
        transitionFunction(c.i, c.j);
        }else{
            pthread_mutex_unlock(&mutex);
             }}
    return NULL;
}
void transitionFunction(int x, int y){  //Funzione di transizione
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
void exchBoard(){   //Scambio bordi fra vicini
    MPI_Status status;
    int c;
    //colonne
    MPI_Send(&readM[v(0,NCOLS/xPartitions)], 1, columnType, rankRight, 17, MPI_COMM_WORLD);
    MPI_Send(&readM[v(0,1)], 1, columnType, rankLeft, 20, MPI_COMM_WORLD);
    MPI_Recv(&readM[v(0,0)], 1, columnType, rankLeft, 17, MPI_COMM_WORLD, &status);
    MPI_Recv(&readM[v(0,NCOLS/xPartitions+1)], 1, columnType, rankRight, 20, MPI_COMM_WORLD, &status);

    //righe
    MPI_Send(&readM[v(NROWS/yPartitions,0)], NCOLS/xPartitions+2, MPI_INT, rankDown, 12, MPI_COMM_WORLD);
	MPI_Send(&readM[v(1,0)], NCOLS/xPartitions+2, MPI_INT, rankUp, 15, MPI_COMM_WORLD);
	MPI_Recv(&readM[v(NROWS/yPartitions+1,0)], NCOLS/xPartitions+2, MPI_INT, rankDown, 15, MPI_COMM_WORLD, &status);
	MPI_Recv(&readM[v(0,0)], NCOLS/xPartitions+2, MPI_INT, rankUp, 12, MPI_COMM_WORLD, &status);
}
void print(int step){
    //I processi mandano la loro porzione al processo 0 per stampare
    if(Rank!=0){
        MPI_Request request;
        MPI_Isend(&readM[v(1,1)], 1, rec, 0, 29, MPI_COMM_WORLD, &request);
    }
    else {
        int dest=1;
        MPI_Status stat;
        printf("Step:  %d \n",step+1);
        for(int i=0; i<yPartitions; i++){
            for(int j=0; j<xPartitions; j++){
                if(i==0 && j==0){
                    for(int c=1; c<NROWS/yPartitions+1; c++){
                        for(int r=1; r<NCOLS/xPartitions+1; r++){
                            bigM[h(c-1,r-1)]=readM[v(c,r)];
                        }
                    }
                }else{
                    MPI_Recv(&bigM[h(i*(NROWS/yPartitions),j*(NCOLS/xPartitions))], 1, bigMtype, dest, 29, MPI_COMM_WORLD, &stat);
                    dest++;
                } 
            }
        } 
    }

    //Stampa di bigM
    if(Rank==0){
        //disegno con allegro sul rank 0
        drawWithAllegro(step);

        //stampo sul terminale senza allegro
        for(int i=0; i<NROWS; i++){
            for(int j=0; j<NCOLS; j++){
                printf("%d ", bigM[h(i, j)]);
            }
            printf("\n");
        }
        printf("-----------------------------------------------\n");
    }
}

void transFunc(){   //funzione di aggiunta task alla thread pool
	for(int i=1;i<NROWS/yPartitions+1;i++){
		for(int j=1;j<NCOLS/xPartitions+1;j++){
            cell c(i,j);
            q.push(c);
            pthread_cond_broadcast(&cond);
			}}
            pthread_create(&threadWait, NULL, &runWait, NULL);
}

void * runWait(void *arg){ //funzione che mette in attesa il processo della fine dei thread
    pthread_mutex_lock(&mutexWait);
    while(!q.empty()){
        pthread_cond_wait(&condWait, &mutexWait);
    }
    pthread_mutex_unlock(&mutexWait);
    return NULL;
}
void swap(){    //swap fra matrici
    pthread_join(threadWait, NULL);
    int * p=readM;
    readM=writeM;
    writeM=p;
}


//----------ALLEGRO----------
//funzione per inizializzare allegro
void initAllegro(){
	//inizializzo allegro
    allegro_init();

	//setto i colori per le altre funzioni
	set_color_depth(24);

	//creo lo schermo in cui vado a disegnare...
	buffer = create_bitmap(WIDTH, HEIGHT);
	//...e definisco di metterla nella finestra
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIDTH, HEIGHT, 0, 0);

	//con queste tre istruzioni vado a settare un titolo alla finestra che vado a creare
    char windowTitle[50];
    sprintf(windowTitle, "Allegro Screen");
    set_window_title(windowTitle);

	//inizializzo i colori bianco e nero
	nero = makecol(0, 0, 0);
	bianco = makecol(255, 255, 255);
}

//Funzione per disegnare
void drawWithAllegro(int step){
	//NOTA: questa funzione si adatta alle varie dimensioni di input ricalcolando sempre altezza e larghezza di ogni blocco in base al numero di righe e colonne

	//qua vado a calcolare larghezza e altezza di ogni singolo rettangolo che vado a disegnare
    int const CELL_WIDTH = WIDTH / NCOLS;
    int const CELL_HEIGHT = HEIGHT / NROWS;

	//qui faccio partire un doppio ciclo for per scorrere la matrice e disegnare.
	for (int i = 0; i < NROWS; i++)
		for (int j = 0; j < NCOLS; j++){
			//calcolo la x e la y iniziale di ogni blocco che devo disegnare
            int x = i * CELL_HEIGHT;
            int y = j * CELL_WIDTH;

            //switch per verificare contenuto della cella della matrice
			switch (bigM[h(i, j)]) {
				case 0:
					//se la cella (i, j) è 0 vado a disegnare un rettangolo nero.
					//parte dalla posizione x, y e si sviluppa in larghezza di y+CELL_WIDTH; si sviluppa in altezza di x+CELL_HEIGHT
					rectfill(buffer, y, x, y + CELL_WIDTH, x + CELL_HEIGHT, nero);
					break;
				case 1:
					//se la cella (i, j) è 1 vado a disegnare un rettangolo bianco.
					//parte dalla posizione x, y e si sviluppa in larghezza di y+CELL_WIDTH; si sviluppa in altezza di x+CELL_HEIGHT
					rectfill(buffer, y, x, y + CELL_WIDTH, x + CELL_HEIGHT, bianco);
					break;
			}
        }

    //visualizzo le step su schermo allegro
    textprintf_ex(buffer, font, 0, 0, bianco, nero, "Step: %d", step);

	//mando il buffer sullo schermo
	blit(buffer, screen, 0, 0, 0, 0, WIDTH, HEIGHT);
}
