#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <shellapi.h>

#define ID_TRAY_ICON 1
#define ID_TRAY_EXIT 2
#define LEFT_ALT 0xA4
#define KEY_Q 0x51
#define KEY_Z 0x5A

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void toggle_window_decorations();
void create_tray_icon(HWND hwnd);

bool running = true;
bool decorations_removed = false;

int main() {
    MSG msg;
    HWND hwnd = CreateWindowEx(
        0,
        "STATIC",
        "windecoration-inator",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        NULL, NULL, NULL
    );

    if (!hwnd) {
        fprintf(stderr, "Failed to create message-only window\n");
        return 1;
    }

    create_tray_icon(hwnd);

    printf("Keybinds:\n");
    printf("\tLEFT ALT + Q: Toggle decorations on active window\n");
    printf("\tLEFT ALT + Z: Exit program\n");

    while (running) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if ((GetAsyncKeyState(LEFT_ALT) & 0x8000) && (GetAsyncKeyState(KEY_Q) & 0x8000)) {
            toggle_window_decorations();
            Sleep(200);
        }

        if ((GetAsyncKeyState(LEFT_ALT) & 0x8000) && (GetAsyncKeyState(KEY_Z) & 0x8000)) {
            printf("Exiting program...\n");
            break;
        }

        Sleep(20);
    }

    Shell_NotifyIcon(NIM_DELETE, &(NOTIFYICONDATA){
        .cbSize = sizeof(NOTIFYICONDATA),
        .hWnd = hwnd,
        .uID = ID_TRAY_ICON,
    });

    return 0;
}

void toggle_window_decorations() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return;

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);

    if (decorations_removed) {
        style |= (WS_CAPTION | WS_SIZEBOX | WS_BORDER);
        decorations_removed = false;
        printf("Restored window decorations.\n");
    } else {
        style &= ~(WS_CAPTION | WS_SIZEBOX | WS_BORDER);
        decorations_removed = true;
        printf("Removed window decorations.\n");
    }

    SetWindowLongPtr(hwnd, GWL_STYLE, style);
    UpdateWindow(hwnd);
}

void create_tray_icon(HWND hwnd) {
    NOTIFYICONDATA nid = {
        .cbSize = sizeof(NOTIFYICONDATA),
        .hWnd = hwnd,
        .uID = ID_TRAY_ICON,
        .uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP,
        .uCallbackMessage = WM_USER + 1,
        .hIcon = LoadIcon(NULL, IDI_APPLICATION),
    };
    snprintf(nid.szTip, sizeof(nid.szTip), "windecoration-inator");

    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
        fprintf(stderr, "ruhroh\n");
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_USER + 1) {
        if (LOWORD(lParam) == WM_RBUTTONUP) {
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Quit");

            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
    } else if (msg == WM_COMMAND) {
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            running = false;
            PostQuitMessage(0);
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
