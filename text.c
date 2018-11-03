#include <stdio.h>
#include <stdarg.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_surface.h>

SDL_Surface *asciialpha;

void text_init(void)
{
    asciialpha = IMG_Load("asciialpha.png");
}

int vsdlprintf(SDL_Surface *surface, int x, int y, char *fmt, va_list argp)
{
    static char buffer[1024];
    vsprintf(buffer, fmt, argp);
    printf("%s\n", buffer);
    char *b = buffer;
    int i = 0;
    while (*b) {
	int sx = *b & 31;
	int sy = *b >> 5;
	SDL_Rect srcrect = { sx*9, sy*16, 9, 16 };
	SDL_Rect dstrect = { x + i*9, y, 9, 32 };
	SDL_BlitSurface(asciialpha, &srcrect, surface, &dstrect);
	b++;
	i++;
    }
}

int sdlprintf(SDL_Surface *surface, int x, int y, char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    int r = vsdlprintf(surface, x, y, fmt, argp);
    va_end(argp);
}
   
