//classe per inizializzare allegro
#ifndef ALLEGRO_INIT_H
#define ALLEGRO_INIT_H

//includo le librerie che mi servono
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

namespace allegro{
    class Init{
        public:
            //qua faccio una init per vedere se posso iniziare a lavorare con allegro
            static void init(){
                if (!al_init() || !al_init_primitives_addon()) {
                    printf("Unable to init allegro!");
                    exit(10);
                }
            }

            //utile per la gestione delle sleep
            static void sleep(float seconds){
                al_rest(seconds);
            }
    };
}

#endif