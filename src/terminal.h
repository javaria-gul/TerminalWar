#ifndef TERMINAL_H
#define TERMINAL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>


bool    terminal_init(SDL_Renderer *renderer, TTF_Font *font);
void terminal_handle_event(SDL_Event* e, SDL_Rect panelRect);

void    terminal_render(SDL_Renderer *renderer, TTF_Font *font, SDL_Rect panelRect);
void    terminal_destroy(void);

#endif // TERMINAL_H

