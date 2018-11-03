#include <stdio.h>
#include <stdarg.h>
#include <SDL.h>
#include <SDL_image.h>


int vsdlprintf(SDL_Surface *surface, char *fmt, va_list argp)
{
    static char buffer[1024];
    vsprintf(buffer, fmt, argp);
    // sdl-flork
   
}

int sdlprintf(SDL_Surface *surface, char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    int r = vsdlprintf(surface, fmt, argp);
    va_end(argp);
}
   
