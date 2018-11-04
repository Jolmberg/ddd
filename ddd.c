#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_surface.h>
#include <math.h>

#include "motherboard.h"
#include "8088.h"
#include "sdl_text.h"
#include "colour.h"

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Texture *debugger;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }
    window = SDL_CreateWindow("Flork", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 240, 0);
    if (!window) {
    //if (SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_SHOWN, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    text_init(renderer);

    debugger = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, 320, 240);
    if (!debugger) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s", SDL_GetError());
        return 3;
    }

    
    printf("asd\n");
    
    float i = 0;
    while (1) {
	i += 0.1;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

	SDL_SetRenderTarget(renderer, debugger);
	SDL_RenderClear(renderer);
	sdlprintf(renderer, 60 + 55 * cos(i/1.5), 120 + 100 * sin(i), &(struct colour){ 0, 0, 128 + 127 * sin(i/10), 150 + 100 * cos(i/9) }, &(struct colour){ 0, 155, 100, 0 }, "broffe!=)(+rk%x", 16777216);
	SDL_SetRenderTarget(renderer, NULL);

        SDL_RenderCopy(renderer, debugger, NULL, NULL);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0x00);
	SDL_RenderDrawLine(renderer, 160 + 120 * cos(i/3), 10 + 200*fabs(cos(i/5+2)), 100 + 90*sin(i/4+2), 100 + 70*cos(i/2));
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
    struct motherboard *mb = mb_create();
    mb_load_bios_rom(mb, "BIOS_5150_24APR81_U33.BIN");
    mb_powerup(mb);
    mb_run(mb);
}
