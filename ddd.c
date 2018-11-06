#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_surface.h>
#include <math.h>
#include <pthread.h>

#include "motherboard.h"
#include "8088.h"
#include "sdl_text.h"
#include "colour.h"
#include "gui.h"

int main(int argc, char *argv[])
{

    if (gui_init() != 0) {
	printf("Disaster!\n");
    }
    
    gui_loop();

    gui_destroy();
    return 0;
}
