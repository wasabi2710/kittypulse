#include <SDL3/SDL.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <windows.h>
#include <stdio.h>

#define PROJECT_NAME "KittyPulse"

SDL_Window* window;
SDL_Renderer* renderer;

typedef struct {
    int width;
    int height;
} ScreenSize;

// helpers will be modulized soon

void logging(const char* msg) { /*need to find away to pass const char -> non const char*/
    fprintf(stderr, "%s", msg);
}

ScreenSize getPrimaryRes() { // get primary display res
    ScreenSize size;
    size.width = GetSystemMetrics(SM_CXSCREEN);
    size.height = GetSystemMetrics(SM_CYSCREEN);
    return size;
}

void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    FILE* file = fopen("log.txt", "w");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        logging(SDL_GetError());
        return 1;
    }
    if(!SDL_CreateWindowAndRenderer(PROJECT_NAME, getPrimaryRes().width, getPrimaryRes().height, SDL_WINDOW_TRANSPARENT | SDL_WINDOW_ALWAYS_ON_TOP, &window, &renderer)) {
        logging(SDL_GetError());
        return 1;
    }

    // set native layering and transparent
    HWND hwnd = GetActiveWindow(); // this window
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    SDL_Texture* sprite = IMG_LoadTexture(renderer, "src/images/Cat Sprite Sheet_1.png");
    if (sprite) {
        SDL_SetTextureScaleMode(sprite, SDL_SCALEMODE_NEAREST); //or SDL_SCALEMODE_BEST
    }
    SDL_FRect dstRect = {500, 500, 128, 128};  // Screen position & size

    SDL_Event e;
    while(1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                cleanup();
                return EXIT_SUCCESS;
            }
        }

        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        /*render*/
        SDL_RenderTexture(renderer, sprite, NULL, &dstRect);

        SDL_RenderPresent(renderer);
    }
    
}