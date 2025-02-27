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
    DeleteObject(hRgnClickable); // Important: Release the region
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
    SetClickableRegion(hwnd, 250, 250, 50, 50); // Center of the window, 200x200 clickable area

    SDL_Event e;
    POINT cursorPos;
    SDL_Point clientCursorPos;

    while (1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                printf("Exit program ...\n");
                return 0; //exit_success
            }
        }

        // Get mouse position using Windows API
        if (GetCursorPos(&cursorPos)) {
            if (ScreenToClient(hwnd, &cursorPos)) {
                clientCursorPos.x = cursorPos.x;
                clientCursorPos.y = cursorPos.y;
                //printf("Mouse position: %d, %d\n", clientCursorPos.x, clientCursorPos.y);
            }
        }

        // Clear the window with a specific color (e.g., black)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw the rectangle with a different color (e.g., green)


        SDL_RenderPresent(renderer);
    }
}