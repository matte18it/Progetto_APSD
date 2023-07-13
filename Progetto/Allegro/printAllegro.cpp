#include "printAllegro.h"

void printAllegro::initAllegro(int Rank, int WIDTH, int HEIGHT){
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

void printAllegro::drawWithAllegro(int NCOLS, int xPartitions, int yPartitions, int NROWS, int WIDTH, int HEIGHT, int* readM, int Rank){
    int const CELL_WIDTH = WIDTH / (NCOLS / xPartitions);
    int const CELL_HEIGHT = HEIGHT / (NROWS / yPartitions);

	for (int i = 1; i < NROWS/yPartitions+1; i++)
		for (int j = 1; j < NCOLS/xPartitions+1; j++){
            int x = (i-1) * CELL_HEIGHT;
            int y = (j-1) * CELL_WIDTH;
            printf("X1:%d   X2:%d   Y1:%d   Y2:%d, i:%d    j:%d    readM:%d   rank:%d\n", x, x+CELL_HEIGHT, y, y+CELL_WIDTH, i, j, readM[v(i, j)], Rank);
            switch (readM[v(i, j)]) {
			case 0:
				rectfill(buffer, y, x, y + CELL_WIDTH, x + CELL_HEIGHT, nero);
				break;
			case 1:
				rectfill(buffer, y, x, y + CELL_WIDTH, x + CELL_HEIGHT, bianco);
				break;
			}
        }
	blit(buffer, screen, 0, 0, 0, 0, WIDTH, HEIGHT);
    readkey();
}