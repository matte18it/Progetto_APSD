#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef PRINT_ALLEGRO_H
#define PRINT_ALLEGRO_H

#define v(r,c) ((r)*(NCOLS/xPartitions+2)+(c))
#define h(r,c) ((r)*(NCOLS)+(c))

class printAllegro {
    private:
        BITMAP *buffer;
        int nero, bianco;

    public:
        void initAllegro(int Rank, int WIDTH, int HEIGHT);
        void drawWithAllegro(int NCOLS, int xPartitions, int yPartitions, int NROWS, int WIDTH, int HEIGHT, int* bigM);
};

#include "printAllegro.cpp"

#endif