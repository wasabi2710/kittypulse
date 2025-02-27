#include <SDL3/SDL.h>
#include <windows.h>
#include <stdio.h>

SDL_Window* window;
SDL_Renderer* renderer;

void SetClickableRegion(HWND hwnd, int centerX, int centerY, int width, int height) {
    int left = centerX - (width / 2);
    int top = centerY - (height / 2);
    int right = centerX + (width / 2);
    int bottom = centerY + (height / 2);

    HRGN hRgnClickable = CreateRectRgn(left, top, right, bottom);
    SetWindowRgn(hwnd, hRgnClickable, TRUE);
    DeleteObject(hRgnClickable);
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Failed Init SDL Video\n");
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("KittyPulse", 500, 500, SDL_WINDOW_ALWAYS_ON_TOP, &window, &renderer)) {
        printf("Failed Window & Renderer Creation\n");
        return 1;
    }

    // set layered
    HWND hwnd = GetActiveWindow();
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Set the clickable region
    SetClickableRegion(hwnd, 250, 250, 50, 50); // Center of the window, 50x50 clickable area.

    SDL_Event e;
    POINT cursorPos;
    SDL_Point clientCursorPos;

    while (1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                printf("Exit program ...\n");
                return 0;
            }
            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                // Check if the click was within the clickable region (the filled rectangle)
                if (e.button.x >= 225 && e.button.x <= 275 && e.button.y >= 225 && e.button.y <= 275) {
                    printf("Hello\n");
                }
            }
        }

        // Get mouse position using Windows API
        if (GetCursorPos(&cursorPos)) {
            if (ScreenToClient(hwnd, &cursorPos)) {
                clientCursorPos.x = cursorPos.x;
                clientCursorPos.y = cursorPos.y;
            }
        }

        // Clear the window with a specific color (e.g., black)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw the larger rectangle with a different color (e.g., green)
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_FRect rect;
        rect.x = 50;
        rect.y = 50;
        rect.w = 100;
        rect.h = 100;
        SDL_RenderRect(renderer, &rect);

        // Draw the smaller filled rectangle in the center
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color
        SDL_FRect fillRect;
        fillRect.x = 225; // Center X - width/2
        fillRect.y = 225; // Center Y - height/2
        fillRect.w = 50;
        fillRect.h = 50;
        SDL_RenderFillRect(renderer, &fillRect);

        SDL_RenderPresent(renderer);
    }
}