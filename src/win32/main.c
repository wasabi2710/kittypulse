#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Draw semi-transparent red circle
            HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));  // Red color
            Ellipse(hdc, 150, 150, 250, 250);  // Draw circle in the center

            // Draw horizontal bars
            RECT rect;
            GetClientRect(hwnd, &rect);
            int barHeight = 20;
            int spacing = 10;

            // Draw bars above circle
            for(int y = 50; y < 150; y += barHeight + spacing) {
                Rectangle(hdc, 100, y, 600, y + barHeight);
            }

            // Draw bars below circle
            for(int y = 250; y < rect.bottom - 50; y += barHeight + spacing) {
                Rectangle(hdc, 100, y, 600, y + barHeight);
            }

            DeleteObject(brush);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            // Get the coordinates of the click
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);

            // Check if the click is inside the circle
            int circleCenterX = 200;
            int circleCenterY = 200;
            int radius = 50;

            int dx = xPos - circleCenterX;
            int dy = yPos - circleCenterY;
            if (dx * dx + dy * dy <= radius * radius) {
                // Print "Hello" if the click is inside the circle
                MessageBox(hwnd, "Hello", "Info", MB_OK);
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    const char CLASS_NAME[] = "Transparent Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    if (!RegisterClass(&wc)) {
        return 0;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED,  // Extended style for transparency
        CLASS_NAME, "Transparent Window",
        WS_POPUP,
        100, 100, 800, 600,
        NULL, NULL, wc.hInstance, NULL
    );

    if (!hwnd) {
        return 0;
    }

    // Set semi-transparency (alpha: 200 = less transparent)
    SetLayeredWindowAttributes(hwnd, 0, 200, LWA_ALPHA);

    HRGN hRegion = CreateEllipticRgn(150, 150, 250, 250);
    SetWindowRgn(hwnd, hRegion, TRUE);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
