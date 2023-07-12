//classe per la gestione del pannello
#ifndef ALLEGRO_PRINTALLEGRO_H
#define ALLEGRO_PRINTALLEGRO_H

//includo le librerie che mi servono e uso il namespace che ho definito prima
#include "allegro/Panel.h"
using namespace allegro;

#define NCOLS 20
#define v(r,c) ((r)*(NCOLS)+(c))

//estendo la classe Panel
class PrintAllegro : public Panel{
    private:
        int *world;            //mi indica la dimensione della mia matrice
        int n; 
        int m;

    public:
        //inizializzo il tutto
        PrintAllegro(int *w, int n1, int m1){
            world = w; n = n1; m = m1;
        }

    protected:
        void paintComponent(Graphics g){
            Panel::paintComponent(g);
            //dimensione di ogni cella
            int size = 50;
            //doppio for per scorrere il mio mondo
            for(int i = 0; i < n; i++){
                for(int j = 0; j < m; j++){
                    //printf("%d ", world[v(i, j)]);
                    //se la mia cella è a 0 la coloro di nero
                    if(world[v(i, j)] == 0)
                        g.setColor(Color::black());
                    //se è a 1, la coloro di bianco
                    else
                        g.setColor(Color::white());

                    //disengo il rettangolo nella posizione corrispondente di dimensione data da size
                    g.fillRect(size*j, size*i, size, size);
                }
                //printf("\n");
            }
        }
};

#endif