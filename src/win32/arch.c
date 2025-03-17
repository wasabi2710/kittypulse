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
NOTIFYICONDATA nid = {0};

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

void updateMovement(char* animationName, SDL_FlipMode* flipMode, State* state, SDL_FRect* dstRect, int* jump, int* onTop) {
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
    static float lastJumpHeight = -1;  // Store last jump height

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

    // Check if cat should jump
    if ((int)dstRect->x >= (actWindLeft) && (int)dstRect->x <= (actWindRight - 128)) {
        if (actWindTop > 128) {
            desiredJumpHeight = getPrimaryRes().height - actWindTop;
            *jump = 1;
            lastJumpHeight = actWindTop;
        } 
    } else if (dstRect->y < desiredJumpHeight) {
        isJumping = 1;
    }

    if (actWindTop < 128 && dstRect->y < desiredJumpHeight) {
        isJumping = 1;
    }

    if (*jump == 1 && !isJumping) {
        velocityY = jumpImpulse;
        isJumping = 1;
        strcpy(animationName, "JUMP");
    }

    // Handle jumping and falling
    if (isJumping) {
        dstRect->y += velocityY * dt;
        velocityY += gravity * dt;

        if (velocityY > 0) {
            strcpy(animationName, "FALL");
        }

        // Ensure cat lands on the window if it is still there
        if (dstRect->y < (actWindTop - 128)) {
            dstRect->y = actWindTop - 128;
            isJumping = 0;
            velocityY = 0;
            strcpy(animationName, "SLEEP");
            *onTop = 1;
        }

        // If window moves and cat is left floating, force fall
        if (lastJumpHeight != -1 && actWindTop > lastJumpHeight && dstRect->y < (actWindTop - 128)) {
            isJumping = 1;  // Continue falling
        }

        // Land on the ground
        if (dstRect->y >= getPrimaryRes().height - 128) {
            dstRect->y = getPrimaryRes().height - 128;
            isJumping = 0;
            velocityY = 0;
            strcpy(animationName, "IDLE_1");
            *onTop = 0;
        }

        *jump = 0;
    }

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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        //tray
        case WM_USER + 1:
            if (lParam == WM_RBUTTONDOWN) {  // Right-click
                POINT cursor;
                GetCursorPos(&cursor);  // Get mouse position

                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 1, "Hide Kitty");
                AppendMenu(hMenu, MF_STRING, 2, "Show Kitty");
                AppendMenu(hMenu, MF_STRING, 3, "Exit");

                SetForegroundWindow(hwnd);  // Prevent menu from auto-closing
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, cursor.x, cursor.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);  // Clean up

                if (cmd == 1) {  // Hide window
                    ShowWindow(hwnd, SW_HIDE);
                }
                if (cmd == 2) {  // Show window
                    ShowWindow(hwnd, SW_SHOW);
                }
                if (cmd == 3) { 
                    cleanup();
                    PostMessage(hwnd, WM_CLOSE, 0, 0); 
                    exit(0);
                }
            }
            break;

        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
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

    // Set the Window Proc callback function for the SDL window
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    //add tray
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    HICON hIcon = (HICON)LoadImage(NULL, "cat.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    nid.hIcon = hIcon;
    
    lstrcpy(nid.szTip, "Kittypulse");
    if(!Shell_NotifyIcon(NIM_ADD, &nid)) {
        MessageBox(NULL, "Failed to add tray icon", "Error", MB_OK);
    }

    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    SDL_Texture* sprite = IMG_LoadTexture(renderer, "src/cat.png");
    SDL_SetTextureScaleMode(sprite, SDL_SCALEMODE_NEAREST);
    SDL_FRect dstRect = {400, getPrimaryRes().height - 128, 128, 128};

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
    int onTop = 0;

    SDL_SetTextureColorMod(sprite, 255, 0, 0); // Apply red tint

    while (1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                SDL_DestroyTexture(sprite);
                cleanup();
                return EXIT_SUCCESS;
            }
        }
        
        if ((SDL_GetTicks() - lastAnimChange) >= 10000) { //10s randomizer
            lastAnimChange = SDL_GetTicks();
            if (strcmp(animationName, "JUMP") != 0 && strcmp(animationName, "FALL") != 0 && onTop != 1) {
                strcpy(animationName, animationStates[rand() % (sizeof(animationStates) / sizeof(animationStates[0]))]);
            }
        }

        updateMovement(animationName, &flipMode, &state, &dstRect, &jump, &onTop);

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
