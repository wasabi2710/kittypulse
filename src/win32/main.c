#include <SDL3/SDL.h>
#include <windows.h>
#include <stdio.h>

SDL_Window* window;
SDL_Renderer* renderer;

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Failed Init SDL Video\n");
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("KittyPulse", 500, 500, SDL_WINDOW_TRANSPARENT | SDL_WINDOW_ALWAYS_ON_TOP, &window, &renderer)) {
        printf("Failed Window & Renderer Creation\n");
        return 1;
    }

    // set layered
    HWND hwnd = GetActiveWindow();
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    SDL_Event e;
    float mouseX, mouseY;

    while (1) {
        if (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                printf("Exit program ...\n");
                return 0; //exit_success
            }

            if (e.type == SDL_EVENT_MOUSE_MOTION) {
                // Track the mouse position
                SDL_GetMouseState(&mouseX, &mouseY);
                printf("Mouse position: %f, %f\n", mouseX, mouseY);
            }
        }

        // Clear the window with a specific color (e.g., black)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw the rectangle with a different color (e.g., green)
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_FRect rect;
        rect.x = rect.y = 50; // Position the rectangle
        rect.w = rect.h = 100; // Size of the rectangle
        SDL_RenderRect(renderer, &rect);
        
        SDL_RenderPresent(renderer);
    }
}
