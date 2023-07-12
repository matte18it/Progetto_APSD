//classe per creare il pannello in cui visualizzare la matrice
#ifndef ALLEGRO_PANEL_H
#define ALLEGRO_PANEL_H

//includo le librerie che mi servono
#include <allegro5/allegro.h>
#include "Color.h"
#include "Graphics.h"

namespace allegro{
    class Panel{
        private:
            Graphics g;                 //mi serve per disegnare
            Color background;           //mi serve per settare il background
            bool chiuso;                //mi serve per indicare se il pannello è chiuso o aperto
        
        public:
            //costruttore del pannello
            Panel(){
                //imposto il background del pannello a nero
                background = Color::black();
                //setto la variabile chiuso (mi indica se il pannello è chiuso) a false
                chiuso = false;
            }

            virtual ~Panel() {}

            void setBackground(Color c){
                background = c;             //cambio il colore di sfondo
            }

            void close(){
                chiuso = true;              //il pannello è chiuso
            }

            bool closed(){
                return chiuso;
            }

            void repaint(){
                //se il pannello è chiuso non faccio niente...
                if(chiuso)
                    return;

                //...altrimentti faccio repaint
                paintComponent(g);
                al_flip_display();
            }

            virtual void paintComponent(Graphics g) {
                al_clear_to_color(background.toAllegro());
            }
    };
}

#endif
