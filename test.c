#include <windows.h>
#include <stdio.h>

int main() {
    HWND lastActive = NULL;
    char lastTitle[256] = {0};

    while (1) {
        HWND active = GetForegroundWindow();
        if (active && active != lastActive) {
            char title[256] = {0};
            if (GetWindowText(active, title, sizeof(title))) {
                if (strcmp(title, lastTitle) != 0) {
                    printf("Active window changed: %s\n", title);
                    strcpy(lastTitle, title);
                }
            }
            lastActive = active;
        }
        Sleep(500); // Check every 500 milliseconds
    }

    return 0;
}
