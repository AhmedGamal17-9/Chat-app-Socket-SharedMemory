#include "Gui.h"
#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    Gui app(hInstance);
    if (!app.CreateMainWindow()) {
        MessageBoxW(NULL, L"Failed to create main window!", L"Error", MB_ICONERROR);
        return 1;
    }

    app.Show(nCmdShow);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}
