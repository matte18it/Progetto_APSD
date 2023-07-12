//Classe che si occupa di disegnare l'interno del mio panel
#ifndef ALLEGRO_GRAPHICS_H
#define ALLEGRO_GRAPHICS_H

//includo le librerie che mi servono
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include "Color.h"

//qui parte la mia classe
namespace allegro{
    class Graphics{
        //contiene il colore con cui vado a disegnare
        private:
            Color color;

        public:
            //funzione per disegnare rettangolo a partire dalle coordinate x e y, di dimensioni x+w (larghezza) e y+h(altezza)
            void fillRect(int x, int y, int w, int h){
                al_draw_filled_rectangle(x, y, x + w, y + h, color.toAllegro());
            }

            //funzione per settare il colore con cui andare a disegnare
            void setColor(Color c) {
                color = c;
            }
    };
}

#endif