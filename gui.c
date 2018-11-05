#include <SDL.h>
#include <pthread.h>

#include "sdl_text.h"
#include "motherboard.h"
#include "8088.h"

SDL_Window *window;
SDL_Renderer *renderer;

struct motherboard *mb;
struct iapx88 *cpu;

int gui_init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }
    window = SDL_CreateWindow("Flork", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 240, 0);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    text_init(renderer);
    
    // Create emulator here for now
    mb = mb_create();
    cpu = mb->cpu;
    mb->debug = 1;
    mb_load_bios_rom(mb, "BIOS_5150_24APR81_U33.BIN");
    mb_powerup(mb);
    //mb_run(mb);

    return 0;    
}

int gui_loop()
{
    SDL_Texture *debugger;
    SDL_Event event;
    pthread_t emulator;
    int tret = pthread_create(&emulator, NULL, mb_run, (void *)mb);
    if(tret)
    {
	fprintf(stderr,"Failed to create thread: %d\n", tret);
	return 100;
    }
    
    debugger = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, 320, 240);
    if (!debugger) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s", SDL_GetError());
        return 3;
    }

    float i = 0;
    while (1) {
	i += 0.1;
        while (SDL_PollEvent(&event)) {
	    if (event.type == SDL_QUIT) {
		return 0;
	    }
	    if (event.type == SDL_KEYUP) {
		printf("mollusk\n");
		if (event.key.keysym.sym == SDLK_c) {
		    printf("RARAJ\n");
		    pthread_mutex_lock(&mb->mutex);
		    pthread_cond_signal(&mb->condition);
		    pthread_mutex_unlock(&mb->mutex);
		}
	    }
        }
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

	SDL_SetRenderTarget(renderer, debugger);
	SDL_RenderClear(renderer);
	sdlprintf(renderer, 25 + 20 * cos(i/1.3), 120 + 100 * sin(i), &(struct colour){ 0, 0, 128 + 127 * sin(i/10), 150 + 100 * cos(i/9) }, &(struct colour){ 0, 155, 100, 0 }, "Double Dragon Debugger v0.0", 16777216);
	SDL_SetRenderTarget(renderer, NULL);

        SDL_RenderCopy(renderer, debugger, NULL, NULL);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0x00);
	SDL_RenderDrawLine(renderer, 160 + 120 * cos(i/3), 10 + 200*fabs(cos(i/5+2)), 100 + 90*sin(i/4+2), 100 + 70*cos(i/2));
        SDL_RenderPresent(renderer);
    }
    return 0;
}

void gui_destroy()
{
    //SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

}
