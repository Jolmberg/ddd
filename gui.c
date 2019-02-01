#include <SDL.h>
#include <pthread.h>
#include <stdio.h>

#include "sdl_text.h"
#include "motherboard.h"
#include "8088.h"
#include "debugger.h"

SDL_Window *window;
SDL_Renderer *renderer;

// These should probably be in a struct
struct motherboard *mb;
struct iapx88 *cpu;
struct debugger *debugger;

int gui_init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }
    window = SDL_CreateWindow("Flork", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 720, 480, 0);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    
    text_init(renderer);
    
    // Create emulator here for now
    mb = mb_create();
    cpu = mb->cpu;
    mb->debug = 0;
    mb_load_bios_rom(mb, "BIOS_5150_24APR81_U33.BIN");
    mb_powerup(mb);
    debugger = debugger_create(mb);
    debugger->breakpoint = 0xfe0b0;
    return 0;    
}

void update_tex_title(SDL_Texture *screen)
{
    static float i = 0;
    static char title[] = "Double Dragon Debugger v0.0";
    SDL_SetRenderTarget(renderer, screen);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    for (int j = 0; j < strlen(title); j++) {
	sdlprintf(renderer,
		  30 + 9 * j -20 * cos(i-j/2.5),
		  120 + 10 * sin(i-j/5.0) + 50 * sin(-(i/10 - j/20.0)),
		  &(struct colour){ 255, 0, 128 + 127 * sin(i/10), 150 + 100 * cos(i/9) },
		  &(struct colour){ 0, 0, 0, 0 }, "%c", title[j]);
    }
    i += 0.1;
}

void init_tex_regview(SDL_Texture *d)
{
    static char *indices[] = { "SP", "BP", "SI", "DI" };
    static char *segments[] = { "ES", "CS", "SS", "DS" };
    SDL_SetRenderTarget(renderer, d);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    for (int i = 0; i < 4; i++) {
	sdlprintf(renderer, 0, 16*i,
		  &(struct colour){ 255, 200, 200, 200},
		  &(struct colour){ 255, 0, 0, 160 - 32*i},
		  " %cX        ", 'A' + i);
	sdlprintf(renderer, 11*9, 16*i,
		  &(struct colour){ 255, 200, 200, 200},
		  &(struct colour){ 255, 160 - 32*i, 0, 0},
		  " %s        ", indices[i]);
	sdlprintf(renderer, 22*9, 16*i,
		  &(struct colour){ 255, 200, 200, 200},
		  &(struct colour){ 255, 0, 160 - 32*i, 0},
		  " %s        ", segments[i]);
    }
    sdlprintf(renderer, 0, 64,
	      &(struct colour){ 255, 200, 200, 200},
	      &(struct colour){ 255, 100, 0, 100},
	      " IP        ");
    /* sdlprintf(renderer, 12*9, 64, */
    /* 	      &(struct colour){ 255, 200, 200, 200}, */
    /* 	      &(struct colour){ 255, 160, 100, 0}, */
    /* 	      " PF                     "); */
    sdlprintf(renderer, 0, 80,
              &(struct colour){ 255, 50, 50, 50},
              &(struct colour){ 255, 120, 120, 0},
              " OF DF IF TF SF ZF AF PF CF ");
}
	
void update_tex_regview(SDL_Texture *texture, struct debugger *debugger)
{
    const int general[4] = { 0, 3, 1, 2 };
    const char flagname[9][3] = { "OF", "DF", "IF", "TF", "SF", "ZF", "AF", "PF", "CF" };
    const int flagpos[9] = { 2048, 1024, 512, 256, 128, 64, 16, 4, 1 };
    struct registers *regs = debugger_get_cpu_regs(debugger, 0);
    struct registers *lastregs = NULL;
    struct iapx88 *cpu = debugger->cpu;
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    if (debugger->register_history_usage > 1) {
        lastregs = debugger_get_cpu_regs(debugger, 1);
    } else {
        lastregs = regs;
    }
    /*     lasgregs = debugger->register_history + debugger->register_history_usage */
    for (int i = 0; i < 4; i++) {
        int j = general[i];
	sdlprintf(renderer, 3*9, 16*i,
		  (regs->reg16[j] == lastregs->reg16[j]) ? &(struct colour){ 255, 200, 200, 200 } : &(struct colour){ 255, 255, 255, 0 },
		  NULL,
		  " 0x%04x ", regs->reg16[general[i]]);
	sdlprintf(renderer, 14*9, 16*i,
		  &(struct colour){ 255, 200, 200, 200 },
		  NULL,
		  " 0x%04x ", regs->reg16[4+i]);
	sdlprintf(renderer, 25*9, 16*i,
		  &(struct colour){ 255, 200, 200, 200 },
		  NULL,
		  " 0x%04x ", regs->segreg[i]);
    }
    sdlprintf(renderer, 3*9, 64,
              &(struct colour){ 255, 200, 200, 200 },
              NULL,
              " 0x%04x ", regs->ip);
    for (int i = 0; i < 9; i++) {
        if (regs->flags & flagpos[i]) {
            sdlprintf(renderer, i*27 + 9, 80,
                      &(struct colour){ 255, 200, 200, 200 },
                      NULL,
                      "%s", flagname[i]);
        }
    }
    for (int i = 0; i < cpu->prefetch_usage; i++) {
        sdlprintf(renderer, i*6*9, 96,
                  &(struct colour){ 255, 200, 200, 200 },
                  &(struct colour){ 255, 0, 160, 160 },
                  " 0x%02x ",
                  cpu->prefetch_queue[(cpu->prefetch_start + i) & 3]);
    }



}

void update_tex_disassembly(SDL_Texture *texture, struct debugger *debugger)
{
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_RenderClear(renderer);
    int lines = debugger->disassembly_lines < 15 ? debugger->disassembly_lines : 15;
    for (int i = 0; i < lines; i++) {
        int current = EA(debugger->cpu->cs, debugger->cpu->ip) == debugger->disassembly_addresses[i];
        sdlprintf(renderer, 0, 16*i,
                  current ? &(struct colour){ 255, 255, 255, 100 } : &(struct colour){ 255, 170, 170, 170 },
                  &(struct colour){ 255, 50, 50, 50 },
                  " 0x%05x: ", debugger->disassembly_addresses[i]);
	int x = sdlprintf(renderer, 11*9, 16*i,
			  current ? &(struct colour){ 255, 255, 255, 100 } : &(struct colour){ 255, 200, 200, 200 },
			  NULL,
			  debugger->disassembly[i]);
	for (int j = 0; j < debugger->lengths[i]; j++) {
	    sdlprintf(renderer, 11*9 + x * 9 + 9 + j*27, 16*i ,
		      &(struct colour){ 255, 100, 100, 100 },
		      NULL,
		      "%02x", debugger->bytes[i][j]);
	}
    }
}


int gui_loop()
{
    SDL_Texture *tex_screen, *tex_regview_bg, *tex_regview_fg, *tex_disassembly;
    SDL_Event event;
    SDL_Rect screen_rect = { 0, 0, 320, 240};
    SDL_Rect rect_regview = { 320, 240, 320, 240};
    SDL_Rect rect_disassembly = { 320, 0, 400, 240};
    pthread_t emulator;
    int last_debugger_step = -2;
    int tret = pthread_create(&emulator, NULL, debugger_run, (void *)debugger);
    if(tret)
    {
	fprintf(stderr,"Failed to create thread: %d\n", tret);
	return 100;
    }
    tex_screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, 320, 240);
    tex_regview_bg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, 320, 240);
    tex_regview_fg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, 320, 240);
    tex_disassembly = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, 400, 240);
    if (!tex_screen || !tex_regview_bg || !tex_regview_fg) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s", SDL_GetError());
        return 3;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(tex_regview_fg, SDL_BLENDMODE_BLEND);
    init_tex_regview(tex_regview_bg);
    
    while (1) {
        while (SDL_PollEvent(&event)) {
	    if (event.type == SDL_QUIT) {
		return 0;
	    }
	    if (event.type == SDL_KEYUP) {
		switch(event.key.keysym.sym) {
		case SDLK_c:
		    pthread_mutex_lock(&mb->mutex);
		    pthread_cond_signal(&mb->condition);
		    pthread_mutex_unlock(&mb->mutex);
		    break;
                case SDLK_r:
                    debugger->paused = 0;
		    pthread_mutex_lock(&mb->mutex);
		    pthread_cond_signal(&mb->condition);
		    pthread_mutex_unlock(&mb->mutex);
                    break;
		case SDLK_q:
		    return 0;
		}
	    }
        }
	update_tex_title(tex_screen);
	/* if (debugger->step != mb->step) { */
	/*     debugger_step(debugger); */
	/*     update_tex_regview(tex_regview_fg, debugger); */
	/*     update_tex_disassembly(tex_disassembly, debugger); */
	/* } */
        if (debugger->step != last_debugger_step) {
            last_debugger_step = debugger->step;
	    update_tex_regview(tex_regview_fg, debugger);
	    update_tex_disassembly(tex_disassembly, debugger);
        }
	SDL_SetRenderTarget(renderer, NULL);
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, tex_screen, NULL, &screen_rect);
	SDL_SetTextureBlendMode(tex_regview_fg, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(renderer, tex_regview_bg, NULL, &rect_regview);
	SDL_RenderCopy(renderer, tex_regview_fg, NULL, &rect_regview);
	SDL_RenderCopy(renderer, tex_disassembly, NULL, &rect_disassembly);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0x00);
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
