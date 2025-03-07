#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define PROJECT_NAME "KittyPulse"
#define GRAV 10.f
#define CATM 100.f
#define GROUND 10000.f

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

/* apply normal newtonian force equation */
float gForce(float dx, float dy) {
    return (GRAV * CATM * GROUND) / (sqrt(dx * dx + dy * dy) / sqrt(dx * dx + dy * dy));
}

void updateMovement(char* animationName, SDL_FlipMode* flipMode, State* state, SDL_FRect* dstRect, int* jump) {
    static float walkSpeed = 5.0f;
    static float runSpeed = 10.0f;
    float dt = 1.f / 60.f;
    static float force;
    static int direction = 1;
    *state = stateMachine(animationName);
    int boundX = getPrimaryRes().width - 128;
    static int isJumping = 0;

    // Jump-related variables
    static float velocityY = 0.0f;  // Vertical velocity
    static const float gravity = 2000.f;  // Gravity force (acceleration)
    static const float desiredJumpHeight = 600.0f;  // Desired jump height in pixels
    static float jumpImpulse = 0.0f;  // Initial jump speed (calculated from desired height)

    // Calculate jumpImpulse based on the desired jump height
    if (!isJumping) {
        jumpImpulse = -sqrt(2 * gravity * desiredJumpHeight);  // Initial jump velocity based on height
    }

    float speed = 0.0f;
    if (strcmp(animationName, "RUN") == 0) {
        speed = runSpeed;
    } else if (strcmp(animationName, "WALK") == 0) {
        speed = walkSpeed;
    }

    dstRect->x += speed * direction;

    // Handle jumping
    if (*jump == 1 && !isJumping) {  // If jump button is pressed and not already jumping
        velocityY = jumpImpulse;  // Apply initial jump velocity
        isJumping = 1;  // Set jumping state
    }

    // Apply gravity when jumping or falling
    if (isJumping) {
        dstRect->y += velocityY * dt;  // Update vertical position based on velocity
        velocityY += gravity * dt;  // Increase downward velocity due to gravity
    }

    printf("y: %f\n", dstRect->y);

    // Clamp the Y position to the ground level
    float groundY = getPrimaryRes().height - 128;
    if (dstRect->y > groundY) {
        dstRect->y = groundY;  // Prevent going below ground
        velocityY = 0.0f;  // Reset vertical velocity
        isJumping = 0;  // Reset jumping state
    }

    // Reset the jump input
    *jump = 0;

    // Horizontal movement and flipping
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
        
        if ((SDL_GetTicks() - lastAnimChange) >= 10000) {
            lastAnimChange = SDL_GetTicks();
            strcpy(animationName, animationStates[rand() % (sizeof(animationStates) / sizeof(animationStates[0]))]);
        }

        updateMovement(animationName, &flipMode, &state, &dstRect, &jump);

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
