#include <SDL3/SDL.h>
#include <stdio.h>

/*init window and renderer*/
SDL_Renderer* renderer;
SDL_Window* window;

void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    /*Check sdl vid init*/
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        // fprintf(stderr, "%s", "Failed: sdl_init_video");
        printf("Failed Video Initialize\n");
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("KittyPulse", 400, 400, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        printf("SDL Window Creation failed\n");
        SDL_Quit();
        return 1;
    }

    /*render draw color: background*/
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    /*event poll*/
    SDL_Event e;

    /*prototypes*/
    SDL_FRect rect;
    rect.x = rect.y = rect.h = rect.w = 100;

    while(1) {
        /*keys event*/
        if (SDL_PollEvent(&e)) {
            /*exit*/
            if (e.type == SDL_EVENT_QUIT) {
                cleanup();
                return SDL_APP_SUCCESS;
            }
            /*key down event*/
            if (e.type == SDL_EVENT_KEY_DOWN) {
                switch (e.key.scancode) {
                    case SDL_SCANCODE_W:
                        rect.x += 1;
                        break;
                    default:
                        break;
                }
            }
        }

        /*render here*/
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &rect);

        /*rendering properties*/
        SDL_RenderPresent(renderer);
        SDL_Delay(16); /*60fps*/
    }   

}