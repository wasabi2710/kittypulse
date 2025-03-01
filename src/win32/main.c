#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3_image/SDL_image.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <dirent.h>

#define PROJECT_NAME "KittyPulse"

SDL_Window* window;
SDL_Renderer* renderer;

/*---------------------- extensions --------------------------*/
typedef struct {
    int width;
    int height;
} ScreenSize;
ScreenSize getPrimaryRes() { // get primary display res
    ScreenSize size;
    size.width = GetSystemMetrics(SM_CXSCREEN);
    size.height = GetSystemMetrics(SM_CYSCREEN);
    return size;
}

/*---------------------- helpers --------------------------*/
void logging(const char* msg) { /*need to find away to pass const char -> non const char*/
    fprintf(stderr, "%s", msg);
}

void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/*---------------------- renderer --------------------------*/
int main() {
    //FILE* file = fopen("log.txt", "w");

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

    SDL_Texture* sprite = IMG_LoadTexture(renderer, "src/cat.png");
    SDL_SetTextureScaleMode(sprite, SDL_SCALEMODE_NEAREST); 
    SDL_FRect dstRect = {500, 500, 128, 128}; // rendering size

    // animation props
    int frameWidth = 32;
    int frameHeight = 32;
    int currentFrame = 0;
    int totalFrames = 8; //config
    int targetRow = 4; //config

    SDL_FRect srcRect;
    srcRect.x = 0;
    srcRect.y = targetRow * frameHeight;
    srcRect.h = frameHeight;
    srcRect.w = frameWidth;

    Uint32 lastFrameTime = SDL_GetTicks();
    float frameDuration = 1000.0f / 10.0f;

    SDL_Event e;
    while(1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                //fclose(file);
                SDL_DestroyTexture(sprite);
                cleanup();
                return EXIT_SUCCESS;
            }

            /*mouse_down inter with texture*/
            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    //       
                }
            }
        }

        // bg color
        srcRect.x = (currentFrame % totalFrames) * frameWidth;

        SDL_RenderClear(renderer);

        /*render*/

        SDL_RenderTexture(renderer, sprite, &srcRect, &dstRect);

        Uint32 currentTicks = SDL_GetTicks();
        if ((currentTicks - lastFrameTime) >= frameDuration) {
            currentFrame++;
            if (currentFrame >= totalFrames) currentFrame = 0;
            lastFrameTime = currentTicks;    
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(32); //cap 30fps
    }
}