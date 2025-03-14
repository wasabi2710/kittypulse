/* prototype: sys tray */
#include <string.h>
#include <windows.h>
#include <shellapi.h>

NOTIFYICONDATA nid = {0};

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
                if (cmd == 3) PostMessage(hwnd, WM_CLOSE, 0, 0);  // Close app
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    HWND hwnd;
    WNDCLASS wc = {0};

    // register class
    wc.lpszClassName = "Test";
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;

    RegisterClass(&wc);

    // create window
    hwnd = CreateWindowExA(
        WS_EX_TOOLWINDOW, // prevent taskbar
        wc.lpszClassName,
        "Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 500,
        NULL, NULL,
        hInstance,
        NULL
    );

    if (!hwnd) return 0;

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

    // show regist hwnd 
    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIconA(NIM_DELETE, &nid);
    return (int)msg.wParam;
}
