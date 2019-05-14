#include <stdio.h>
#include <stdarg.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_surface.h>

#include "colour.h"

SDL_Texture *asciialpha;

void text_init(SDL_Renderer *renderer)
{
    SDL_Surface *s;
    
    s = IMG_Load("asciialpha.png");
    asciialpha = SDL_CreateTextureFromSurface(renderer, s);
    SDL_FreeSurface(s);
}

int vsdlprintf(SDL_Renderer *renderer, int x, int y, struct colour *fg, struct colour *bg, char *fmt, va_list argp)
{
    static char buffer[1024];
    vsprintf(buffer, fmt, argp);
    char *b = buffer;
    int i = 0;
    SDL_SetTextureColorMod(asciialpha, fg->r, fg->g, fg->b);
    if (bg) {
        SDL_SetRenderDrawColor(renderer, bg->r, bg->g, bg->b, bg->a);
    }
    while (*b) {
        int sx = *b & 31;
        int sy = *b >> 5;
        SDL_Rect srcrect = { sx*9, sy*16, 9, 16 };
        SDL_Rect dstrect = { x + i*9, y, 9, 16 };
        if (bg) {
            SDL_RenderFillRect(renderer, &dstrect);
        }
        SDL_RenderCopy(renderer, asciialpha, &srcrect, &dstrect);
        b++;
        i++;
    }
    return i;
}

int sdlprintf(SDL_Renderer *renderer, int x, int y, struct colour *fg, struct colour *bg, char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    int r = vsdlprintf(renderer, x, y, fg, bg, fmt, argp);
    va_end(argp);
    return r;
}
   
