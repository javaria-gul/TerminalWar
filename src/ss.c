#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int TYPING_SPEED_MS = 40;
const int TERMINAL_FONT_SIZE = 20;

bool showInstructionPanel = false;
bool showHintPanel = false;
bool showTerminalPanel = false;

bool isInside(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x <= (rect.x + rect.w) && y >= rect.y && y <= (rect.y + rect.h);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    SDL_Window* window = SDL_CreateWindow("Terminal War",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Fonts
    TTF_Font* titleFont     = TTF_OpenFont("assets/fonts/Neon.ttf", 100);
    TTF_Font* dialogueFont  = TTF_OpenFont("assets/fonts/dialogue_box_font.TTF", 28);
    TTF_Font* terminalFont  = TTF_OpenFont("assets/fonts/arial.ttf", TERMINAL_FONT_SIZE);

    // Textures
    SDL_Texture* bgTexture          = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/FinalBackground.png"));
    SDL_Texture* characterTexture   = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/Character.png"));
    SDL_Texture* iconInstruction    = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/icon_instruction.png"));
    SDL_Texture* iconHint           = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/icon_hint.png"));
    SDL_Texture* iconTerminal       = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/icon_terminal.png"));
    SDL_Texture* textboxTexture     = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/textbox.png"));

    Mix_Music* bgMusic = Mix_LoadMUS("assets/music/music.mp3");

    // Title setup (unchanged)...
    SDL_Color white = {255,255,255,255};
    SDL_Surface* tmp = TTF_RenderText_Solid(titleFont, "TERMINAL WAR", white);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, tmp);
    SDL_Rect titleRect = { SCREEN_WIDTH/2 - tmp->w/2, SCREEN_HEIGHT/2 - tmp->h/2, tmp->w, tmp->h };
    SDL_FreeSurface(tmp);

    // Dialogue setup (unchanged)...
    SDL_Rect characterRect = {-200,420,200,300};
    const char* fullDialogue = "Hello Agent! ... Be prepared.";
    char dialogueBuffer[1024] = "";
    int dialogueLength = strlen(fullDialogue), currentChar=0;
    Uint32 lastTyped = SDL_GetTicks();
    SDL_Rect dialogueBox = {0, SCREEN_HEIGHT-180, SCREEN_WIDTH,150};
    SDL_Rect dialogueRect = {250, SCREEN_HEIGHT-160, SCREEN_WIDTH-280,100};

    // Icon rects
    SDL_Rect iconInstructionRect = {50, 30, 80,80};
    SDL_Rect iconHintRect        = {50,130, 80,80};
    SDL_Rect iconTerminalRect    = {50,230, 80,80};

    // Terminal state
    char termInput[256] = "";
    int  termLen = 0;
    char *termOutput[128]; memset(termOutput,0,sizeof(termOutput));
    int  termLines = 0;

    bool quit=false, fadingIn=true, showTitleFlag=true, bgMusicStarted=false, characterSlideDone=false;
    int mouseX, mouseY, alpha=0;
    SDL_StartTextInput();

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_QUIT) quit=true;
            else if (e.type==SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&mouseX,&mouseY);
                if (isInside(mouseX,mouseY,iconInstructionRect)) {
                    showInstructionPanel = !showInstructionPanel; showHintPanel = showTerminalPanel = false;
                } else if (isInside(mouseX,mouseY,iconHintRect)) {
                    showHintPanel = !showHintPanel; showInstructionPanel = showTerminalPanel = false;
                } else if (isInside(mouseX,mouseY,iconTerminalRect)) {
                    showTerminalPanel = !showTerminalPanel; showInstructionPanel = showHintPanel = false;
                }
            }
            else if (e.type==SDL_TEXTINPUT && showTerminalPanel) {
                if (termLen + strlen(e.text.text) < sizeof(termInput)-1) {
                    strcat(termInput, e.text.text);
                    termLen = strlen(termInput);
                }
            }
            else if (e.type==SDL_KEYDOWN && showTerminalPanel) {
                if (e.key.keysym.sym==SDLK_BACKSPACE && termLen>0) {
                    termInput[--termLen] = '\0';
                } else if (e.key.keysym.sym==SDLK_RETURN) {
                    // run command via popen
                    FILE* fp = popen(termInput,"r");
                    termLines = 0;
                    while (fgets(dialogueBuffer,sizeof(dialogueBuffer),fp) && termLines<128) {
                        termOutput[termLines++] = strdup(dialogueBuffer);
                    }
                    pclose(fp);
                    termLen=0; termInput[0]=0;
                }
            }
        }

        SDL_GetMouseState(&mouseX,&mouseY);
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        // Title fade (unchanged)...
        if (showTitleFlag) {
            SDL_SetTextureAlphaMod(titleTexture,alpha);
            SDL_RenderCopy(renderer,titleTexture,NULL,&titleRect);
            if (fadingIn) { alpha+=3; if(alpha>=255){fadingIn=false; SDL_Delay(1000);} }
            else        { alpha-=3; if(alpha<=0) showTitleFlag=false; }
        }
        else {
            // Background + music
            SDL_RenderCopy(renderer,bgTexture,NULL,NULL);
            if(!bgMusicStarted){Mix_PlayMusic(bgMusic,-1);bgMusicStarted=true;}

            // Dialogue box + character (unchanged)...
            SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,30,30,30,180);
            SDL_RenderFillRect(renderer,&dialogueBox);
            if(!characterSlideDone){
                if(characterRect.x<50) characterRect.x+=8;
                else characterSlideDone=true;
            }
            SDL_RenderCopy(renderer,characterTexture,NULL,&characterRect);
            if(characterSlideDone && currentChar<dialogueLength){
                Uint32 now=SDL_GetTicks();
                if(now-lastTyped > TYPING_SPEED_MS){
                    dialogueBuffer[currentChar]=fullDialogue[currentChar++];
                    lastTyped=now;
                }
            }
            SDL_Surface* ds = TTF_RenderText_Blended_Wrapped(dialogueFont,dialogueBuffer,(SDL_Color){255,105,180,255},dialogueRect.w);
            SDL_Texture* dt = SDL_CreateTextureFromSurface(renderer,ds);
            SDL_FreeSurface(ds);
            SDL_RenderCopy(renderer,dt,NULL,&dialogueRect);
            SDL_DestroyTexture(dt);

            // Icons + hover (unchanged)...
            float scale=1.1f;
            SDL_Rect iR=iconInstructionRect,hR=iconHintRect,tR=iconTerminalRect;
            if(isInside(mouseX,mouseY,iconInstructionRect)){ iR.w*=scale; iR.h*=scale; iR.x-=(iR.w-80)/2; iR.y-=(iR.h-80)/2; }
            if(isInside(mouseX,mouseY,iconHintRect))       { hR.w*=scale; hR.h*=scale; hR.x-=(hR.w-80)/2; hR.y-=(hR.h-80)/2; }
            if(isInside(mouseX,mouseY,iconTerminalRect))   { tR.w*=scale; tR.h*=scale; tR.x-=(tR.w-80)/2; tR.y-=(tR.h-80)/2; }
            SDL_RenderCopy(renderer,iconInstruction,NULL,&iR);
            SDL_RenderCopy(renderer,iconHint,NULL,&hR);
            SDL_RenderCopy(renderer,iconTerminal,NULL,&tR);

            // Instruction/Hint panels (unchanged)...

            // ── Terminal panel ─────────────────────────────────────────────────
            if (showTerminalPanel) {
                // panel area matches instructions positioning
                SDL_Rect termPanel = {
                    dialogueRect.x,
                    40,
                    SCREEN_WIDTH - dialogueRect.x - 40,
                    dialogueBox.y - 50
                };
                // background
                SDL_SetRenderDrawColor(renderer,0,0,0,255);
                SDL_RenderFillRect(renderer,&termPanel);

                // header bar
                SDL_Rect hdr = { termPanel.x, termPanel.y, termPanel.w, TERMINAL_FONT_SIZE+10 };
                SDL_SetRenderDrawColor(renderer,50,50,50,255);
                SDL_RenderFillRect(renderer,&hdr);
                // prompt text
                char prompt[64];
                snprintf(prompt,sizeof(prompt),"TerminalWar@Level-0:~$ ");
                SDL_Surface* ps = TTF_RenderText_Solid(terminalFont,prompt,white);
                SDL_Texture* pt = SDL_CreateTextureFromSurface(renderer,ps);
                SDL_Rect pr = { hdr.x+5, hdr.y+2, ps->w, ps->h };
                SDL_RenderCopy(renderer,pt,NULL,&pr);
                SDL_FreeSurface(ps); SDL_DestroyTexture(pt);

                // user input
                SDL_Surface* is = TTF_RenderText_Solid(terminalFont,termInput,white);
                SDL_Texture* it = SDL_CreateTextureFromSurface(renderer,is);
                SDL_Rect irt = { pr.x+pr.w, hdr.y+2, is->w, is->h };
                SDL_RenderCopy(renderer,it,NULL,&irt);
                SDL_FreeSurface(is); SDL_DestroyTexture(it);

                // output lines
                int yoff = hdr.y + TERMINAL_FONT_SIZE + 15;
                for(int i=0;i<termLines;i++){
                    SDL_Surface* os = TTF_RenderText_Solid(terminalFont, termOutput[i], white);
                    SDL_Texture* ot = SDL_CreateTextureFromSurface(renderer,os);
                    SDL_Rect ort = { termPanel.x+5, yoff, os->w, os->h };
                    SDL_RenderCopy(renderer,ot,NULL,&ort);
                    yoff += TERMINAL_FONT_SIZE + 2;
                    SDL_FreeSurface(os); SDL_DestroyTexture(ot);
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    // Cleanup all...
    Mix_FreeMusic(bgMusic);
    SDL_DestroyTexture(titleTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(characterTexture);
    SDL_DestroyTexture(iconInstruction);
    SDL_DestroyTexture(iconHint);
    SDL_DestroyTexture(iconTerminal);
    SDL_DestroyTexture(textboxTexture);
    TTF_CloseFont(titleFont);
    TTF_CloseFont(dialogueFont);
    TTF_CloseFont(terminalFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}

