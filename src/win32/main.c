#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3_image/SDL_image.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>

#define PROJECT_NAME "KittyPulse"

SDL_Window* window;
SDL_Renderer* renderer;

/*---------------------- extensions --------------------------*/
typedef struct {
    int width;
    int height;
} ScreenSize;

ScreenSize getPrimaryRes() {
    ScreenSize size;
    size.width = GetSystemMetrics(SM_CXSCREEN);
    size.height = GetSystemMetrics(SM_CYSCREEN);
    return size;
}

typedef struct {
    int totalAnims;
    int targetRow;
} State;
State stateMachine(char* state) {
    State currentState;
    if (strcmp(state, "Idle") == 0) {
        currentState.totalAnims = 4;
        currentState.targetRow = 0;
    } else if (strcmp(state, "Walk") == 0) {
        currentState.totalAnims = 8;
        currentState.targetRow = 4;
    }

    return currentState;
}

/*---------------------- helpers --------------------------*/
void logging(const char* msg) {
    fprintf(stderr, "%s", msg);
}

void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/*---------------------- renderer --------------------------*/
int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        logging(SDL_GetError());
        return 1;
    }
    if (!SDL_CreateWindowAndRenderer(PROJECT_NAME, getPrimaryRes().width, getPrimaryRes().height, SDL_WINDOW_TRANSPARENT | SDL_WINDOW_ALWAYS_ON_TOP, &window, &renderer)) {
        logging(SDL_GetError());
        return 1;
    }

    HWND hwnd = GetActiveWindow();
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    SDL_Texture* sprite = IMG_LoadTexture(renderer, "src/cat.png");
    SDL_SetTextureScaleMode(sprite, SDL_SCALEMODE_NEAREST);
    SDL_FRect dstRect = {500, 500, 128, 128};

    // Initialize state properly
    State state = stateMachine("Idle");

    int frameWidth = 32, frameHeight = 32, currentFrame = 0;
    SDL_FRect srcRect = {0, state.targetRow * frameHeight, frameWidth, frameHeight};

    Uint32 lastFrameTime = SDL_GetTicks();
    float frameDuration = 1000.0f / 10.0f;

    SDL_Event e;
    while (1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                SDL_DestroyTexture(sprite);
                cleanup();
                return EXIT_SUCCESS;
            }

            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
                state = stateMachine("Walk");
            }
        }

        // Update srcRect based on current state
        srcRect.x = (currentFrame % state.totalAnims) * frameWidth;
        srcRect.y = state.targetRow * frameHeight;

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, sprite, &srcRect, &dstRect);

        Uint32 currentTicks = SDL_GetTicks();
        if ((currentTicks - lastFrameTime) >= frameDuration) {
            currentFrame = (currentFrame + 1) % state.totalAnims;
            lastFrameTime = currentTicks;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(32); // Cap 30fps
    }
}
