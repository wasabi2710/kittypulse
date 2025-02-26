#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            // Drawing a red circle (clickable area)
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));  // Red color
            Ellipse(hdc, 150, 150, 250, 250);  // Draw circle in the center
            DeleteObject(brush);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    // Register Window Class
    const char CLASS_NAME[] = "Transparent Window Class";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        return 0;
    }

    // Create the Window
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW,  // Extended styles for transparency
        CLASS_NAME, "Transparent Window",
        WS_POPUP,  // No border, full transparency
        100, 100, 800, 600,  // Window size
        NULL, NULL, wc.hInstance, NULL
    );

    if (!hwnd) {
        return 0;
    }

    // Set transparency (alpha: 128 = 50% transparent)
    SetLayeredWindowAttributes(hwnd, 0, 128, LWA_COLORKEY | LWA_ALPHA);

    // Create a clickable region (circle)
    HRGN hRegion = CreateEllipticRgn(150, 150, 250, 250);  // Ellipse in the center
    SetWindowRgn(hwnd, hRegion, TRUE);  // Set the region

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Event Loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
