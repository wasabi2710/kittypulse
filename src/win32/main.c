#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3_image/SDL_image.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

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
    RECT workArea;
    if (SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0)) {
        size.height = workArea.bottom - workArea.top;
    }
    return size;
}

typedef struct {
    char* name;
    int totalAnims;
    int targetRow;
} State;

State stateMachine(char* state) {
    State animationState[] = {
        {"IDLE_1", 4, 0},
        {"IDLE_2", 4, 1},
        {"IDLE_3", 4, 2},
        {"IDLE_4", 4, 3},
        {"WALK", 8, 4},
        {"RUN", 8, 5},   
        {"SLEEP", 4, 6},   
        {"HIT", 6, 7},     
        {"SCARED", 7, 8},  
        {"FRIGHT", 8, 9},  
    };
    
    for (size_t i = 0; i < sizeof(animationState) / sizeof(animationState[0]); i++) {
        if(strcmp(state, animationState[i].name) == 0) {
            return animationState[i];
        }
    }
    return animationState[0];
}

void updateMovement(char* animationName, SDL_FlipMode* flipMode, State* state, SDL_FRect* dstRect) {
    static float walkSpeed = 5.0f;
    static float runSpeed = 10.0f;
    static int direction = 1;
    *state = stateMachine(animationName);
    int boundX = getPrimaryRes().width - 128;
    
    float speed = 0.0f;
    if (strcmp(animationName, "RUN") == 0) {
        speed = runSpeed;
    } else if (strcmp(animationName, "WALK") == 0) {
        speed = walkSpeed;
    }

    dstRect->x += speed * direction;
    if (dstRect->x <= 0) {
        *flipMode = SDL_FLIP_NONE;
        direction = 1;
    } else if (dstRect->x >= boundX) {
        *flipMode = SDL_FLIP_HORIZONTAL;
        direction = -1;
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

    char* animationStates[] = {"IDLE_1", "IDLE_2", "IDLE_3", "IDLE_4", "WALK", "RUN", "HIT", "SCARED", "FRIGHT"};
    char animationName[10];
    strcpy(animationName, "IDLE_1");
    State state = stateMachine(animationName);
    
    int frameWidth = 32, frameHeight = 32, currentFrame = 0;
    SDL_FRect srcRect = {0, state.targetRow * frameHeight, frameWidth, frameHeight};

    Uint32 lastFrameTime = SDL_GetTicks();
    float frameDuration = 1000.0f / 10.0f;
    Uint32 lastAnimChange = SDL_GetTicks();
    
    SDL_Event e;
    SDL_FlipMode flipMode = SDL_FLIP_NONE;

    while (1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                SDL_DestroyTexture(sprite);
                cleanup();
                return EXIT_SUCCESS;
            }
        }
        
        if ((SDL_GetTicks() - lastAnimChange) >= 10000) {
            lastAnimChange = SDL_GetTicks();
            strcpy(animationName, animationStates[rand() % (sizeof(animationStates) / sizeof(animationStates[0]))]);
        }

        updateMovement(animationName, &flipMode, &state, &dstRect);

        srcRect.x = (currentFrame % state.totalAnims) * frameWidth;
        srcRect.y = state.targetRow * frameHeight;

        SDL_RenderClear(renderer);
        SDL_RenderTextureRotated(renderer, sprite, &srcRect, &dstRect, 0, 0, flipMode);

        Uint32 currentTicks = SDL_GetTicks();
        if ((currentTicks - lastFrameTime) >= frameDuration) {
            currentFrame = (currentFrame + 1) % state.totalAnims;
            lastFrameTime = currentTicks;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(32);
    }
}
