#pragma once
#include "Network.h" // Must be before windows.h
#include <windows.h>
#include <string>
#include "SharedMemory.h"

#define ID_BTN_SOCKET_MODE 101
#define ID_BTN_SHM_MODE    102
#define ID_BTN_SERVER      103
#define ID_BTN_CLIENT      104
#define ID_BTN_START_NET   105
#define ID_EDIT_IP         106
#define ID_EDIT_PORT       107
#define ID_BTN_PERSON_A    108
#define ID_BTN_PERSON_B    109
#define ID_EDIT_NAME_A     110
#define ID_EDIT_NAME_B     111
#define ID_TOGGLE_WRITE    112
#define ID_LIST_LOGS       113
#define ID_EDIT_SEND       114
#define ID_BTN_SEND        115
#define ID_LIST_CHAT       116

enum AppMode {
    MODE_SELECTION,
    MODE_SOCKET,
    MODE_SHM
};

class Gui {
public:
    Gui(HINSTANCE hInstance);
    ~Gui();

    bool CreateMainWindow();
    void Show(int nCmdShow);
    
    // Called from Message Loop
    LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void Update(); // Called periodically by timer or loop

private:
    void ClearScreen();
    void DrawModeSelection();
    void DrawSocketMode();
    void DrawShmMode();

    void OnNetStart();
    void OnShmSend();
    void OnNetSend();

    HINSTANCE m_hInstance;
    HWND m_hWnd;
    HFONT m_hFont;

    AppMode m_currentMode = MODE_SELECTION;
    
    // UI Controls Handles
    std::vector<HWND> m_controls;

    // Logic Managers
    NetworkManager m_network;
    SharedMemoryManager m_shm;

    // State
    bool m_isServer = false;
    bool m_shmIsPersonB = false;
};
