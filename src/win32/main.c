#include <SDL3/SDL.h>
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
    RECT workAea;
    if (SystemParametersInfo(SPI_GETWORKAREA, 0, &workAea, 0)) {
        size.height = workAea.bottom - workAea.top;
    }

    return size;
}

typedef struct {
    int totalAnims;
    int targetRow;
} State;
State stateMachine(char* state) {
    State currentState;
    if (strcmp(state, "IDLE") == 0) {
        currentState.totalAnims = 4;
        currentState.targetRow = 0;
    } else if (strcmp(state, "WALK") == 0) {
        currentState.totalAnims = 8;
        currentState.targetRow = 4;
    } else if (strcmp(state, "RUN") == 0) {
        currentState.totalAnims = 8;
        currentState.targetRow = 5;
    }

    return currentState;
}

void updateMovement(char* animationName, State* state, SDL_FRect* dstRect) {
    
    /* mod main state */
    *state = stateMachine(animationName);

    /* mod main movement */
    if (strcmp(animationName, "IDLE") == 0) {
        dstRect->x = 0.f;
    } else if (strcmp(animationName, "WALK") == 0) {
        dstRect->x += 1.f;
    } else if (strcmp(animationName, "RUN") == 0) {
        dstRect->x += 3.f;
    }

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
    SDL_FRect dstRect = {0, getPrimaryRes().height - 128, 128, 128};

    // Initialize state properly
    char animationName[10];
    strcpy(animationName, "IDLE");
    State state = stateMachine(animationName);

    int frameWidth = 32, frameHeight = 32, currentFrame = 0;
    SDL_FRect srcRect = {0, state.targetRow * frameHeight, frameWidth, frameHeight};

    Uint32 lastFrameTime = SDL_GetTicks();
    float frameDuration = 1000.0f / 10.0f;

    int swap = 1;

    SDL_Event e;
    while (1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                SDL_DestroyTexture(sprite);
                cleanup();
                return EXIT_SUCCESS;
            }

            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
                if (e.button.x >= dstRect.x && e.button.x <= (dstRect.x + dstRect.w) &&
                    e.button.y >= dstRect.y && e.button.y <= (dstRect.y + dstRect.h)) {
                    
                    // prototype: changing state based on movement
                    if (swap == 1) {
                        strcpy(animationName, "WALK");
                        swap = 2; // Switch to RUN next time
                    } else {
                        strcpy(animationName, "RUN");
                        swap = 1; // Switch back to WALK next time
                    }
                }
            }
        }

        // update movement and state
        updateMovement(animationName, &state, &dstRect);

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
