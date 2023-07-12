//Classe che uso per cambiare colore
#ifndef ALLEGRO_COLOR_H
#define ALLEGRO_COLOR_H

//includo le librerie che mi servono
#include <allegro5/allegro.h>

namespace allegro{
    class Color{
        //variabile privata che contiene il colore
        private:
            ALLEGRO_COLOR color;

        public:
            //costruttore di base in cui inizializzo il colore a nero
            Color(){
                color = al_map_rgb(0, 0, 0);
            }

            //serve per cambiare colore, stampa un errore qualore ci fosse un valore non valido
            Color(unsigned int r, unsigned int g, unsigned int b){
                color = al_map_rgb(0, 0, 0);
                if(r > 255 || g > 255 || b > 255){
                    printf("This color doesn’t exist!");
                    return;
                }
                color = al_map_rgb(r, g, b);
            }

            //funzione per cambiare colore in nero (corrisponde allo 0 della matrice)
            static Color black(){
                return Color(0, 0, 0);
            }

            //funzione per cambiare colore in bianco (corrisponde all'1 nella matrice)
            static Color white(){
                return Color(255, 255, 255);
            }

            //ritorna il colore
            ALLEGRO_COLOR toAllegro() {
                return color;
            }
    };
}

#endif