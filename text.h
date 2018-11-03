#ifndef _TEXT_H_
#define _TEXT_H_

void text_init(void);
int vsdlprintf(SDL_Surface *surface, int x, int y, char *fmt, va_list argp);
int sdlprintf(SDL_Surface *surface, int x, int y, char *fmt, ...);

#endif
