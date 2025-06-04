#include "terminal.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char    inputBuf[1024] = "";
static int     inputLen = 0;
static SDL_Texture *outputTex = NULL;
static SDL_Renderer *gRenderer = NULL;

// Initialize: start text input
bool terminal_init(SDL_Renderer *renderer, TTF_Font *font) {
    gRenderer = renderer;
    SDL_StartTextInput();
    return true;
}

// Handle typing/backspace/enter
void terminal_handle_event(SDL_Event* e, SDL_Rect panelRect) {
    
    if (e->type == SDL_TEXTINPUT && inputLen < sizeof(inputBuf)-1) {
        strcat(inputBuf, e->text.text);
        inputLen = strlen(inputBuf);
    }
    else if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_BACKSPACE && inputLen > 0) {
            inputBuf[--inputLen] = '\0';
        }
        else if (e->key.keysym.sym == SDLK_RETURN) {
            // run via popen
            FILE *pipe = popen(inputBuf, "r");
            char out[8192] = "";
            if (pipe) {
                fread(out,1,sizeof(out)-1,pipe);
                pclose(pipe);
            }
            // render output to texture
            // you may choose your color
            SDL_Color col = {0,255,0,255};
            extern TTF_Font* dialogueFont; // or pass font pointer
            extern SDL_Renderer* gRenderer;
            SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(
                // pass same font as dialogueFont
                TTF_OpenFont("assets/fonts/dialogue_box_font.TTF",28),
                out, col,  panelRect.w - 20
            );
            if (outputTex) SDL_DestroyTexture(outputTex);
            outputTex = SDL_CreateTextureFromSurface(gRenderer, surf);
            SDL_FreeSurface(surf);
            // clear inputBuf
            inputBuf[0] = '\0';
            inputLen = 0;
        }
    }
}

// Draw prompt + previous output
void terminal_render(SDL_Renderer *renderer, TTF_Font *font, SDL_Rect panelRect) {
    char prompt[256];
    snprintf(prompt,sizeof(prompt),"TerminalWar@Level-0:$ %s",inputBuf);
    SDL_Color col = {0,255,0,255};
    SDL_Surface* psurf = TTF_RenderText_Blended(font, prompt, col);
    SDL_Texture* ptex  = SDL_CreateTextureFromSurface(renderer, psurf);
    SDL_Rect pr = { panelRect.x+10, panelRect.y+10, psurf->w, psurf->h };
    SDL_FreeSurface(psurf);
    SDL_RenderCopy(renderer,ptex,NULL,&pr);
    SDL_DestroyTexture(ptex);

    if (outputTex) {
        SDL_Rect or = {
            panelRect.x+10,
            panelRect.y + 20 + TTF_FontHeight(font),
            panelRect.w - 20,
            panelRect.h - (20 + TTF_FontHeight(font) + 10)
        };
        SDL_RenderCopy(renderer, outputTex, NULL, &or);
    }
}

void terminal_destroy(void) {
    SDL_StopTextInput();
    if (outputTex) SDL_DestroyTexture(outputTex);
}

