#ifndef _TEXT_H_
#define _TEXT_H_

#include "colour.h"

void text_init(SDL_Renderer *renderer);
int vsdlprintf(SDL_Renderer *renderer, int x, int y, struct colour *fg, struct colour *bg, char *fmt, va_list argp);
int sdlprintf(SDL_Renderer *renderer, int x, int y, struct colour *fg, struct colour *bg, char *fmt, ...);

#endif
