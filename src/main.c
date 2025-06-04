// src/main.c

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "terminal.h"  // اگر بعد میں terminal.sh کال کرنا ہے تو یہ شامل رہے گا

// Screen dimensions اور typing effect کا سپیڈ
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int TYPING_SPEED_MS = 40;
const Uint32 DIALOGUE_HIDE_DELAY = 5000; // 5 سیکنڈ

// مختلف UI کے لیے flags
bool showInstructionPanel = false;
bool showHintPanel        = false;
bool showTerminalPanel    = false;
bool dialogueEnded        = false;
Uint32 dialogueEndTime    = 0;
bool showPasswordBox      = false;  // password box کب دکھائی دے گی
bool passwordActive       = false;  // آیا user password field میں لکھ رہا ہے

char passwordText[64]     = "";      // password input buffer
Uint32 lastCursorToggle   = 0;
bool   showCursor         = true;    // blink کرنے کے لیے

// Helper: چیک کرنے کے لیے آیا mouse کسی rect کے اندر ہے
bool isInside(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x <= rect.x + rect.w
        && y >= rect.y && y <= rect.y + rect.h;
}

// Helper: text render کر کے texture لوٹائے
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font,
                        const char* text, SDL_Color color) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

int main(int argc, char* argv[]) {
    // SDL, SDL_image, SDL_ttf, SDL_mixer initialization
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) < 0) return 1;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return 1;
    if (TTF_Init() == -1) return 1;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return 1;

    // Window اور Renderer بنائیں
    SDL_Window* window = SDL_CreateWindow("Terminal War",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return 1;
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return 1;

    // Fonts load کریں
    TTF_Font* titleFont    = TTF_OpenFont("assets/fonts/Neon.ttf", 100);
    TTF_Font* dialogueFont = TTF_OpenFont("assets/fonts/dialogue_box_font.TTF", 28);
    if (!titleFont || !dialogueFont) return 1;

    // Textures load کریں: background, character, icons, textbox (for panel کے لیے)
    SDL_Texture* bgTexture        = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/FinalBackground.png"));
    SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/Character.png"));
    SDL_Texture* iconInst         = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/icon_instruction.png"));
    SDL_Texture* iconHint         = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/icon_hint.png"));
    SDL_Texture* iconTerm         = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/icon_terminal.png"));
    SDL_Texture* textboxTex       = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/images/textbox.png"));
    Mix_Music*   bgMusic          = Mix_LoadMUS("assets/music/music.mp3");

    // Title text تیار کریں
    SDL_Color white = {255,255,255,255};
    SDL_Surface* tsurf = TTF_RenderText_Solid(titleFont, "TERMINAL WAR", white);
    SDL_Texture*  titleTex = SDL_CreateTextureFromSurface(renderer, tsurf);
    int titleW = tsurf->w, titleH = tsurf->h;
    SDL_FreeSurface(tsurf);
    SDL_Rect titleRect = { SCREEN_WIDTH/2 - titleW/2, SCREEN_HEIGHT/2 - titleH/2, titleW, titleH };

    // Character slide-in کیلئے start position
    SDL_Rect characterRect = { -200, 420, 200, 300 };

    // Dialogue setup
    SDL_Color hotPink = {255,105,180,255};
    const char* fullDialogue =
      "Hello Agent! I am Javaria Gul, your game partner. Nice to meet you. "
      "We are about to begin your mission in the Terminal War. Be prepared.";
    char dialogueBuffer[1024] = "";
    int dialogueLength = strlen(fullDialogue), currentChar = 0;
    Uint32 lastTypedTime = SDL_GetTicks();
    SDL_Rect dialogueBox  = { 0, SCREEN_HEIGHT - 180, SCREEN_WIDTH, 150 };
    SDL_Rect dialogueRect = { 250, SCREEN_HEIGHT - 160, SCREEN_WIDTH - 280, 100 };

    // Icons کے لیے rects (size 80x80)
    SDL_Rect iconInstRect = {50, 30, 80, 80};
    SDL_Rect iconHintRect = {50, 130, 80, 80};
    SDL_Rect iconTermRect = {50, 230, 80, 80};

    // Password UI
    SDL_Rect pwdBoxRect     = { SCREEN_WIDTH - 360, 20, 300, 40 };
    SDL_Rect submitBtnRect  = { SCREEN_WIDTH - 110,  20, 100, 40 };  // width بڑھا دیا

    // Submit اور password label کا placeholder؛ ہم dynamic render کریں گے
    SDL_Texture* submitLabelTex = renderText(renderer, dialogueFont, "Submit", white);
    int subW, subH;
    SDL_QueryTexture(submitLabelTex, NULL, NULL, &subW, &subH);
    SDL_Rect submitLabelRect = { submitBtnRect.x + (submitBtnRect.w - subW)/2,
                                 submitBtnRect.y + (submitBtnRect.h - subH)/2,
                                 subW, subH };

    // Terminal subsystem initialize کریں (بعد میں shell کھولیں گے)
    terminal_init(renderer, dialogueFont);

    // Main loop کے flags
    bool quit = false;
    bool fadingIn = true, showTitleFlag = true, bgMusicStarted = false, charSlideDone = false;
    SDL_Event e;
    int mouseX = 0, mouseY = 0, alpha = 0;

    // Enable text input (password کیلئے)
    SDL_StartTextInput();

    while (!quit) {
        Uint32 nowTime = SDL_GetTicks();
        // Cursor blink ہر 500ms پر
        if (nowTime - lastCursorToggle > 500) {
            showCursor = !showCursor;
            lastCursorToggle = nowTime;
        }

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&mouseX, &mouseY);
                // Icons click handling
                if (isInside(mouseX, mouseY, iconInstRect)) {
                    showInstructionPanel = !showInstructionPanel;
                    showHintPanel = showTerminalPanel = false;
                    passwordActive = false;
                }
                else if (isInside(mouseX, mouseY, iconHintRect)) {
                    showHintPanel = !showHintPanel;
                    showInstructionPanel = showTerminalPanel = false;
                    passwordActive = false;
                }
                else if (isInside(mouseX, mouseY, iconTermRect)) {
                    showTerminalPanel = !showTerminalPanel;
                    showInstructionPanel = showHintPanel = false;
                    passwordActive = false;
                }
                // Submit button click (آگے logic add کیا جائے گا)
                else if (showPasswordBox && isInside(mouseX, mouseY, submitBtnRect)) {
                    // TODO: Password validation logic یہاں ڈالیں
                }
                // Password box پر click کر کے active toggle کریں
                else if (showPasswordBox && isInside(mouseX, mouseY, pwdBoxRect)) {
                    passwordActive = true;
                } else {
                    passwordActive = false;
                }
                // Instruction panel close (X)
                if (showInstructionPanel) {
                    SDL_Rect panel = { dialogueRect.x, 40,
                                       SCREEN_WIDTH - dialogueRect.x - 40,
                                       dialogueBox.y - 50 };
                    SDL_Rect crossInst = { panel.x + panel.w - 30, panel.y + 10, 20, 20 };
                    if (isInside(mouseX, mouseY, crossInst)) {
                        showInstructionPanel = false;
                    }
                }
                // Hint panel close (X)
                if (showHintPanel) {
                    SDL_Rect panel = { dialogueRect.x, 40,
                                       SCREEN_WIDTH - dialogueRect.x - 40,
                                       dialogueBox.y - 50 };
                    SDL_Rect crossHint = { panel.x + panel.w - 30, panel.y + 10, 20, 20 };
                    if (isInside(mouseX, mouseY, crossHint)) {
                        showHintPanel = false;
                    }
                }
            }
            // Text input event (password کے لیے)
            else if (e.type == SDL_TEXTINPUT && passwordActive) {
                if (strlen(passwordText) + strlen(e.text.text) < sizeof(passwordText) - 1) {
                    strcat(passwordText, e.text.text);
                }
            }
            else if (e.type == SDL_KEYDOWN && passwordActive) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(passwordText) > 0) {
                    passwordText[strlen(passwordText) - 1] = '\0';
                }
            }
            // Terminal panel event forward کریں
            if (showTerminalPanel) {
                terminal_handle_event(&e, pwdBoxRect);
            }
        }

        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Title animation دکھائیں
        if (showTitleFlag) {
            SDL_SetTextureAlphaMod(titleTex, alpha);
            SDL_RenderCopy(renderer, titleTex, NULL, &titleRect);
            if (fadingIn) {
                alpha += 3;
                if (alpha >= 255) {
                    fadingIn = false;
                    SDL_Delay(1000);
                }
            } else {
                alpha -= 3;
                if (alpha <= 0) showTitleFlag = false;
            }
        } else {
            // Background اور music چلائیں
            SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
            if (!bgMusicStarted) {
                Mix_PlayMusic(bgMusic, -1);
                bgMusicStarted = true;
            }

            // Dialogue box draw کریں
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 180);
            SDL_RenderFillRect(renderer, &dialogueBox);

            // Character slide in
            if (!charSlideDone) {
                if (characterRect.x < 50) characterRect.x += 8;
                else {
                    charSlideDone = true;
                    showPasswordBox = true; // جب character slide ہو جائے تو password دِکھائیں
                }
            }
            SDL_RenderCopy(renderer, characterTexture, NULL, &characterRect);

            // Typing effect + hide after 5 سیکنڈ
            if (charSlideDone) {
                if (currentChar < dialogueLength) {
                    Uint32 now = SDL_GetTicks();
                    if (now - lastTypedTime > TYPING_SPEED_MS) {
                        dialogueBuffer[currentChar] = fullDialogue[currentChar];
                        currentChar++;
                        lastTypedTime = now;
                    }
                } else if (!dialogueEnded) {
                    dialogueEndTime = SDL_GetTicks();
                    dialogueEnded = true;
                } else if (SDL_GetTicks() - dialogueEndTime > DIALOGUE_HIDE_DELAY) {
                    memset(dialogueBuffer, 0, sizeof(dialogueBuffer));
                }
            }
            SDL_Surface* dsurf = TTF_RenderText_Blended_Wrapped(dialogueFont,
                                    dialogueBuffer, hotPink, dialogueRect.w);
            SDL_Texture* dtex = SDL_CreateTextureFromSurface(renderer, dsurf);
            SDL_FreeSurface(dsurf);
            SDL_RenderCopy(renderer, dtex, NULL, &dialogueRect);
            SDL_DestroyTexture(dtex);

            // Hover-zoom icons
            float scale = 1.1f;
            SDL_Rect inst = iconInstRect, hint = iconHintRect, term = iconTermRect;
            if (isInside(mouseX, mouseY, inst)) {
                inst.w *= scale; inst.h *= scale;
                inst.x -= (inst.w - iconInstRect.w) / 2;
                inst.y -= (inst.h - iconInstRect.h) / 2;
            }
            if (isInside(mouseX, mouseY, hint)) {
                hint.w *= scale; hint.h *= scale;
                hint.x -= (hint.w - iconHintRect.w) / 2;
                hint.y -= (hint.h - iconHintRect.h) / 2;
            }
            if (isInside(mouseX, mouseY, term)) {
                term.w *= scale; term.h *= scale;
                term.x -= (term.w - iconTermRect.w) / 2;
                term.y -= (term.h - iconTermRect.h) / 2;
            }
            SDL_RenderCopy(renderer, iconInst, NULL, &inst);
            SDL_RenderCopy(renderer, iconHint, NULL, &hint);
            SDL_RenderCopy(renderer, iconTerm, NULL, &term);

            // Panel area بنائیں اور Instruction/Hints/Terminal دکھائیں
            SDL_Rect panel = {
                dialogueRect.x,
                40,
                SCREEN_WIDTH - dialogueRect.x - 40,
                dialogueBox.y - 50
            };
            // Instructions Panel
            if (showInstructionPanel) {
                // Background: navy blue
                SDL_SetRenderDrawColor(renderer, 10, 20, 70, 220);
                SDL_RenderFillRect(renderer, &panel);
                // Outer border: hot pink
                SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255);
                SDL_RenderDrawRect(renderer, &panel);
                // Inner border: white (inset by 3 px)
                SDL_Rect inner = { panel.x+3, panel.y+3, panel.w-6, panel.h-6 };
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &inner);
                // Close (X) button upper-right of panel
                SDL_Rect crossInst = { panel.x + panel.w - 30, panel.y + 10, 20, 20 };
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(renderer, crossInst.x, crossInst.y,
                                   crossInst.x + crossInst.w, crossInst.y + crossInst.h);
                SDL_RenderDrawLine(renderer, crossInst.x + crossInst.w, crossInst.y,
                                   crossInst.x, crossInst.y + crossInst.h);
                // یہاں آپ dummy text ڈال سکتے ہیں
            }
            // Hints Panel
            if (showHintPanel) {
                SDL_SetRenderDrawColor(renderer, 10, 20, 70, 220);
                SDL_RenderFillRect(renderer, &panel);
                SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255);
                SDL_RenderDrawRect(renderer, &panel);
                SDL_Rect inner = { panel.x+3, panel.y+3, panel.w-6, panel.h-6 };
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &inner);
                SDL_Rect crossHint = { panel.x + panel.w - 30, panel.y + 10, 20, 20 };
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(renderer, crossHint.x, crossHint.y,
                                   crossHint.x + crossHint.w, crossHint.y + crossHint.h);
                SDL_RenderDrawLine(renderer, crossHint.x + crossHint.w, crossHint.y,
                                   crossHint.x, crossHint.y + crossHint.h);
            }
            // Terminal Panel
            if (showTerminalPanel) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &panel);
                terminal_render(renderer, dialogueFont, panel);
            }
        }

        // ———— Password box & Submit button ————
        if (showPasswordBox) {
            // Password box background (white semi-transparent)
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 230);
            SDL_RenderFillRect(renderer, &pwdBoxRect);
            // Border (hot pink)
            SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255);
            SDL_RenderDrawRect(renderer, &pwdBoxRect);

            // Render entered password (asterisks)
            char stars[64] = "";
            for (size_t i = 0; i < strlen(passwordText); ++i) {
                strcat(stars, "*");
            }
            SDL_Texture* pwdTex = renderText(renderer, dialogueFont, stars, white);
            int pwW, pwH = TTF_FontHeight(dialogueFont);
            SDL_QueryTexture(pwdTex, NULL, NULL, &pwW, NULL);
            SDL_Rect pwdTextRect = { pwdBoxRect.x + 10, pwdBoxRect.y + (pwdBoxRect.h - pwH)/2, pwW, pwH };
            SDL_RenderCopy(renderer, pwdTex, NULL, &pwdTextRect);
            SDL_DestroyTexture(pwdTex);

            // Cursor blink (height forced to font height)
            if (passwordActive && showCursor) {
                SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
                SDL_Rect cursorRect = { pwdTextRect.x + pwW + 2, pwdTextRect.y, 2, pwH };
                SDL_RenderFillRect(renderer, &cursorRect);
            }

            // Submit button background (hover effect)
            if (isInside(mouseX, mouseY, submitBtnRect)) {
                SDL_SetRenderDrawColor(renderer, 255, 20, 147, 255); // hover hot pink (darker)
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255); // normal hot pink
            }
            SDL_RenderFillRect(renderer, &submitBtnRect);
            // Border (white)
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &submitBtnRect);
            // Submit label
            SDL_RenderCopy(renderer, submitLabelTex, NULL, &submitLabelRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Cleanup
    SDL_StopTextInput();
    terminal_destroy();
    SDL_DestroyTexture(titleTex);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(characterTexture);
    SDL_DestroyTexture(iconInst);
    SDL_DestroyTexture(iconHint);
    SDL_DestroyTexture(iconTerm);
    SDL_DestroyTexture(textboxTex);
    SDL_DestroyTexture(submitLabelTex);
    Mix_FreeMusic(bgMusic);
    TTF_CloseFont(titleFont);
    TTF_CloseFont(dialogueFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}

