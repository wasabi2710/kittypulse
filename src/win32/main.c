#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <windef.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define PROJECT_NAME "KittyPulse"

SDL_Window* window;
SDL_Renderer* renderer;

typedef struct {
    int width;
    int height;
} ScreenSize;

ScreenSize getPrimaryRes() {
    ScreenSize size = {0, 0}; // Initialize all fields
    size.width = GetSystemMetrics(SM_CXSCREEN);

    RECT workArea;
    if (SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0)) {
        size.height = workArea.bottom - workArea.top;
    } else {
        size.height = GetSystemMetrics(SM_CYSCREEN); // Fallback if SPI_GETWORKAREA fails
    }

    return size;
}


typedef struct {
    char* name;
    int start;
    int end;
    int targetRow;
    int totalFrames;
} State;

State stateMachine(char* state) {
    State animationState[] = {
        {"IDLE_1", 0, 3, 0, 4},
        {"IDLE_2", 0, 3, 1, 4},
        {"IDLE_3", 0, 3, 2, 4},
        {"IDLE_4", 0, 3, 3, 4},
        {"WALK", 0, 7, 4, 8},
        {"RUN", 0, 7, 5, 8},
        {"SLEEP", 0, 3, 6, 4},
        {"HIT", 0, 5, 7, 6},
        {"FRIGHT", 0, 7, 9, 8},
        {"JUMP", 0, 1, 8, 2},
        {"FALL", 2, 6, 8, 5},
    };

    for (size_t i = 0; i < sizeof(animationState) / sizeof(animationState[0]); i++) {
        if(strcmp(state, animationState[i].name) == 0) {
            return animationState[i];
        }
    }
    return animationState[0];
}

void updateMovement(char* animationName, SDL_FlipMode* flipMode, State* state, SDL_FRect* dstRect, int* jump) {
    static float walkSpeed = 5.0f;
    static float runSpeed = 10.0f;
    float dt = 1.f / 60.f;
    static int direction = 1;
    *state = stateMachine(animationName);
    int boundX = getPrimaryRes().width - 128;
    static int isJumping = 0;
    static float velocityY = 0.0f;
    static const float gravity = 2000.f;
    static float desiredJumpHeight = 200.0f;
    static float jumpImpulse = 0.0f;
    static int activeWindowTopAtJumpStart = 0; // Store the top of the active window when jumping.

    if (!isJumping) {
        jumpImpulse = -sqrt(2 * gravity * desiredJumpHeight);
    }

    float speed = 0.0f;
    if (strcmp(animationName, "RUN") == 0) {
        speed = runSpeed;
    } else if (strcmp(animationName, "WALK") == 0) {
        speed = walkSpeed;
    }

    dstRect->x += speed * direction;

    // Get active window's top position
    HWND actWind = GetForegroundWindow();
    int actWindTop = 0;
    int actWindBot = 0;
    int actWindLeft = 0;
    int actWindRight = 0;
    if (actWind) {
        RECT actWindRect;
        if (GetWindowRect(actWind, &actWindRect)) {
            actWindTop = actWindRect.top;
            actWindBot = actWindRect.bottom;
            actWindLeft = actWindRect.left;
            actWindRight = actWindRect.right;
        }
    }

    // Check if cat's y position is lower (greater in value) than the window top
    if (actWindTop > 128) {
        desiredJumpHeight = getPrimaryRes().height - actWindTop;
        *jump = 1;
    }

    if (actWindTop < 128) {
        isJumping = 1;
    }

    if (*jump == 1 && !isJumping) {
        velocityY = jumpImpulse;
        isJumping = 1;
        strcpy(animationName, "JUMP");
    }

    if (isJumping) {
        dstRect->y += velocityY * dt;
        velocityY += gravity * dt;

        if (velocityY > 0) {
            strcpy(animationName, "FALL");
        }

        // Check for landing on active window
        if (actWindTop > 128 && dstRect->y < (actWindTop - 128)) {
            dstRect->y = actWindTop - 128;
            isJumping = 0;
            velocityY = 0;
            strcpy(animationName, "IDLE_1");
        } 
    
        if (dstRect->y >= getPrimaryRes().height - 128) {
            dstRect->y = getPrimaryRes().height - 128;  // Land on the bottom of the screen
            isJumping = 0;
            velocityY = 0;
            strcpy(animationName, "IDLE_1");
        }

    }

    *jump = 0;

    if (dstRect->x <= 0) {
        *flipMode = SDL_FLIP_NONE;
        direction = 1;
    } else if (dstRect->x >= boundX) {
        *flipMode = SDL_FLIP_HORIZONTAL;
        direction = -1;
    }
}

void logging(const char* msg) {
    fprintf(stderr, "%s", msg);
}

void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}   

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
    State state;

    int frameWidth = 32, frameHeight = 32, currentFrame = 0;
    SDL_FRect srcRect = {0, state.targetRow * frameHeight, frameWidth, frameHeight};

    Uint32 lastFrameTime = SDL_GetTicks();
    float frameDuration = 1000.0f / 10.0f;
    Uint32 lastAnimChange = SDL_GetTicks();
    
    SDL_Event e;
    SDL_FlipMode flipMode = SDL_FLIP_NONE;
    int jump = 0;

    while (1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                SDL_DestroyTexture(sprite);
                cleanup();
                return EXIT_SUCCESS;
            }

            if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    jump = 1;
                }
            }
        }

        //printf("dimensions: left %d and rigth %d\n", getPrimaryRes().left, getPrimaryRes().right);
        // printf("dst->y %f\n", dstRect.y);
        
        if ((SDL_GetTicks() - lastAnimChange) >= 3000) {
            lastAnimChange = SDL_GetTicks();
            if (strcmp(animationName, "JUMP") != 0 && strcmp(animationName, "FALL") != 0) {
                strcpy(animationName, animationStates[rand() % (sizeof(animationStates) / sizeof(animationStates[0]))]);
            }
        }

        updateMovement(animationName, &flipMode, &state, &dstRect, &jump);

        srcRect.x = currentFrame * frameWidth;
        srcRect.y = state.targetRow * frameHeight;

        SDL_RenderClear(renderer);
        SDL_RenderTextureRotated(renderer, sprite, &srcRect, &dstRect, 0, 0, flipMode);

        if (strcmp(animationName, "JUMP") != 0 && strcmp(animationName, "FALL") != 0) {
            Uint32 currentTicks = SDL_GetTicks();
            if ((currentTicks - lastFrameTime) >= frameDuration) {
                currentFrame = (currentFrame + 1) % state.totalFrames;
                lastFrameTime = currentTicks;
            }
        } else {
            currentFrame = state.totalFrames - 1;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(32);
    }
}
