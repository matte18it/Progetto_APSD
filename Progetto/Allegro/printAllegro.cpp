#include "printAllegro.h"

void printAllegro::initAllegro(int Rank, int WIDTH, int HEIGHT){
    allegro_init();
	set_color_depth(24);
	buffer = create_bitmap(WIDTH, HEIGHT);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIDTH, HEIGHT, 0, 0);

    char windowTitle[50];
    sprintf(windowTitle, "Allegro Screen");
    set_window_title(windowTitle);

	nero = makecol(0, 0, 0);
	bianco = makecol(255, 255, 255);
}

void printAllegro::drawWithAllegro(int NCOLS, int xPartitions, int yPartitions, int NROWS, int WIDTH, int HEIGHT, int* bigM){
    int const CELL_WIDTH = WIDTH / NCOLS;
    int const CELL_HEIGHT = HEIGHT / NROWS;

	for (int i = 0; i < NROWS; i++)
		for (int j = 0; j < NCOLS; j++){
            int x = i * CELL_HEIGHT;
            int y = j * CELL_WIDTH;

			//stampa per debug (la uso all'occorrenza)
            //printf("X1:%d   X2:%d   Y1:%d   Y2:%d, i:%d    j:%d    bigM:%d   rank:%d\n", x, x+CELL_HEIGHT, y, y+CELL_WIDTH, i, j, bigM[v(i, j)], 0);
            
			switch (bigM[h(i, j)]) {
			case 0:
				rectfill(buffer, y, x, y + CELL_WIDTH, x + CELL_HEIGHT, nero);
				break;
			case 1:
				rectfill(buffer, y, x, y + CELL_WIDTH, x + CELL_HEIGHT, bianco);
				break;
			}
        }
	blit(buffer, screen, 0, 0, 0, 0, WIDTH, HEIGHT);
}