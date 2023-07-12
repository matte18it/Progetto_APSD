//classe per definire il frame
#ifndef ALLEGRO_FRAME_H
#define ALLEGRO_FRAME_H

//includo le librerie che mi servono
#include <allegro5/allegro5.h>
#include "Panel.h"

namespace allegro{

    class Frame{
        private:
            ALLEGRO_DISPLAY* display;
            Panel *panel;
            bool exit;

        public:
            //creo il frame, passandogli la dimensione e il titolo
            Frame(unsigned int width, unsigned int height, const char* title) {
                //crea il display con le dimensioni passate
                display = al_create_display(width, height);
                //setto il titolo
                al_set_window_title(display, title);
                //qua lo inizializzo a colore nero
                al_clear_to_color(Color::black().toAllegro());
                setExitOnClose(true);
            }

            virtual ~Frame() {
                if(display != nullptr)
                al_destroy_display(display);
            }

            void setExitOnClose(bool value) {
                exit = value;
            }

            void add(Panel *panel) {
                this->panel = panel;
            }

            bool exitOnClose() {
                return exit;
            }
    };
}

#endif
