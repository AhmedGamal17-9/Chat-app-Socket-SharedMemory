#include "Gui.h"
#include "Logger.h"
#include <algorithm>

// Helpers
void AppendText(HWND hList, const std::wstring& text) {
    SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)text.c_str());
    int count = SendMessageW(hList, LB_GETCOUNT, 0, 0);
    SendMessageW(hList, LB_SETTOPINDEX, count - 1, 0);
}

Gui::Gui(HINSTANCE hInstance) : m_hInstance(hInstance), m_hWnd(NULL) {
    m_hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
}

Gui::~Gui() {
    DeleteObject(m_hFont);
}

bool Gui::CreateMainWindow() {
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
        Gui* pGui = (Gui*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (pGui) return pGui->HandleMessage(hWnd, message, wParam, lParam);
        return DefWindowProcW(hWnd, message, wParam, lParam);
    };
    wcex.hInstance = m_hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"ChatSuiteClass";

    if (!RegisterClassExW(&wcex)) return false;

    m_hWnd = CreateWindowExW(0, L"ChatSuiteClass", L"Desktop Communication Suite",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 800, 600, NULL, NULL, m_hInstance, NULL);

    if (!m_hWnd) return false;

    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
    
    DrawModeSelection();
    
    // Timer for updates (logging, messages)
    SetTimer(m_hWnd, 1, 100, NULL);
    
    return true;
}

void Gui::Show(int nCmdShow) {
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
}

void Gui::ClearScreen() {
    for (HWND hFn : m_controls) {
        DestroyWindow(hFn);
    }
    m_controls.clear();
    InvalidateRect(m_hWnd, NULL, TRUE);
}

void Gui::DrawModeSelection() {
    ClearScreen();
    m_currentMode = MODE_SELECTION;
    
    HWND hBtn1 = CreateWindowW(L"BUTTON", L"Socket Chat (LAN)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        250, 200, 300, 50, m_hWnd, (HMENU)ID_BTN_SOCKET_MODE, m_hInstance, NULL);
    SendMessageW(hBtn1, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hBtn1);

    HWND hBtn2 = CreateWindowW(L"BUTTON", L"Shared Memory Chat (Local)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        250, 270, 300, 50, m_hWnd, (HMENU)ID_BTN_SHM_MODE, m_hInstance, NULL);
    SendMessageW(hBtn2, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hBtn2);
}

// ... Helper to create controls ...
HWND CreateCtrl(const wchar_t* cls, const wchar_t* txt, int x, int y, int w, int h, HWND parent, HMENU id, HINSTANCE inst, HFONT font) {
    HWND hwndCtrl = CreateWindowW(cls, txt, WS_VISIBLE | WS_CHILD | WS_BORDER, x, y, w, h, parent, id, inst, NULL);
    SendMessageW(hwndCtrl, WM_SETFONT, (WPARAM)font, TRUE);
    return hwndCtrl;
}

void Gui::DrawSocketMode() {
    ClearScreen();
    m_currentMode = MODE_SOCKET;

    // Controls
    m_controls.push_back(CreateCtrl(L"STATIC", L"Role:", 20, 20, 50, 25, m_hWnd, NULL, m_hInstance, m_hFont));
    
    HWND hServer = CreateWindowW(L"BUTTON", L"Server", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 80, 20, 80, 25, m_hWnd, (HMENU)ID_BTN_SERVER, m_hInstance, NULL);
    SendMessageW(hServer, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hServer);
    
    HWND hClient = CreateWindowW(L"BUTTON", L"Client", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 170, 20, 80, 25, m_hWnd, (HMENU)ID_BTN_CLIENT, m_hInstance, NULL);
    SendMessageW(hClient, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hClient);
    
    // IP/Port
    m_controls.push_back(CreateCtrl(L"STATIC", L"IP:", 270, 20, 30, 25, m_hWnd, NULL, m_hInstance, m_hFont));
    m_controls.push_back(CreateCtrl(L"EDIT", L"127.0.0.1", 310, 20, 150, 25, m_hWnd, (HMENU)ID_EDIT_IP, m_hInstance, m_hFont));

    m_controls.push_back(CreateCtrl(L"STATIC", L"Port:", 480, 20, 40, 25, m_hWnd, NULL, m_hInstance, m_hFont));
    m_controls.push_back(CreateCtrl(L"EDIT", L"8888", 530, 20, 80, 25, m_hWnd, (HMENU)ID_EDIT_PORT, m_hInstance, m_hFont));

    m_controls.push_back(CreateCtrl(L"BUTTON", L"Start", 630, 20, 80, 25, m_hWnd, (HMENU)ID_BTN_START_NET, m_hInstance, m_hFont));

    // Chat Area
    HWND hList = CreateWindowW(L"LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
        20, 60, 740, 300, m_hWnd, (HMENU)ID_LIST_CHAT, m_hInstance, NULL);
    SendMessageW(hList, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hList);

    // Input Area
    m_controls.push_back(CreateCtrl(L"EDIT", L"", 20, 380, 640, 30, m_hWnd, (HMENU)ID_EDIT_SEND, m_hInstance, m_hFont));
    m_controls.push_back(CreateCtrl(L"BUTTON", L"Send", 670, 380, 90, 30, m_hWnd, (HMENU)ID_BTN_SEND, m_hInstance, m_hFont));

    // Logs
    m_controls.push_back(CreateCtrl(L"STATIC", L"System Logs:", 20, 420, 200, 20, m_hWnd, NULL, m_hInstance, m_hFont));
    HWND hLogs = CreateWindowW(L"LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL,
        20, 440, 740, 100, m_hWnd, (HMENU)ID_LIST_LOGS, m_hInstance, NULL);
    SendMessageW(hLogs, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hLogs);

    // Default selection
    SendMessage(hServer, BM_SETCHECK, BST_CHECKED, 0);
    m_isServer = true;
}

void Gui::DrawShmMode() {
    ClearScreen();
    m_currentMode = MODE_SHM;

    // Controls for Shm
    m_controls.push_back(CreateCtrl(L"STATIC", L"I am:", 20, 20, 50, 25, m_hWnd, NULL, m_hInstance, m_hFont));

    HWND hPersonA = CreateWindowW(L"BUTTON", L"Person A", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 80, 20, 100, 25, m_hWnd, (HMENU)ID_BTN_PERSON_A, m_hInstance, NULL);
    SendMessageW(hPersonA, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hPersonA);

    HWND hPersonB = CreateWindowW(L"BUTTON", L"Person B", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 190, 20, 100, 25, m_hWnd, (HMENU)ID_BTN_PERSON_B, m_hInstance, NULL);
    SendMessageW(hPersonB, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hPersonB);

    // Name Fields
    m_controls.push_back(CreateCtrl(L"STATIC", L"Name A:", 310, 20, 60, 25, m_hWnd, NULL, m_hInstance, m_hFont));
    m_controls.push_back(CreateCtrl(L"EDIT", L"User A", 380, 20, 100, 25, m_hWnd, (HMENU)ID_EDIT_NAME_A, m_hInstance, m_hFont));

    m_controls.push_back(CreateCtrl(L"STATIC", L"Name B:", 500, 20, 60, 25, m_hWnd, NULL, m_hInstance, m_hFont));
    m_controls.push_back(CreateCtrl(L"EDIT", L"User B", 570, 20, 100, 25, m_hWnd, (HMENU)ID_EDIT_NAME_B, m_hInstance, m_hFont));

    // Active Writer Switch
    HWND hToggle = CreateWindowW(L"BUTTON", L"Take Control (Writer)", WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_PUSHLIKE | BS_AUTOCHECKBOX, 
        20, 55, 200, 30, m_hWnd, (HMENU)ID_TOGGLE_WRITE, m_hInstance, NULL);
    SendMessageW(hToggle, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hToggle);
    
    // Chat Area
    HWND hList = CreateWindowW(L"LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
        20, 100, 740, 260, m_hWnd, (HMENU)ID_LIST_CHAT, m_hInstance, NULL);
    SendMessageW(hList, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hList);

    // Input Area
    m_controls.push_back(CreateCtrl(L"EDIT", L"", 20, 380, 640, 30, m_hWnd, (HMENU)ID_EDIT_SEND, m_hInstance, m_hFont));
    m_controls.push_back(CreateCtrl(L"BUTTON", L"Send", 670, 380, 90, 30, m_hWnd, (HMENU)ID_BTN_SEND, m_hInstance, m_hFont));

    // Logs
     m_controls.push_back(CreateCtrl(L"STATIC", L"System Logs:", 20, 420, 200, 20, m_hWnd, NULL, m_hInstance, m_hFont));
    HWND hLogs = CreateWindowW(L"LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL,
        20, 440, 740, 100, m_hWnd, (HMENU)ID_LIST_LOGS, m_hInstance, NULL);
    SendMessageW(hLogs, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    m_controls.push_back(hLogs);

    // Initialize logic
    m_shm.Initialize(); 
}

LRESULT Gui::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_TIMER:
            Update();
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == ID_BTN_SOCKET_MODE) DrawSocketMode();
            if (wmId == ID_BTN_SHM_MODE) DrawShmMode();
            
            if (wmId == ID_BTN_SERVER) m_isServer = true;
            if (wmId == ID_BTN_CLIENT) m_isServer = false;

            if (wmId == ID_BTN_START_NET) OnNetStart();
            if (wmId == ID_BTN_SEND) {
                if (m_currentMode == MODE_SOCKET) OnNetSend();
                if (m_currentMode == MODE_SHM) OnShmSend();
            }

            if (wmId == ID_BTN_PERSON_A) {
                m_shm.SetIdentity(false);
                Logger::instance().Log(L"Switched to Person A");
            }
            if (wmId == ID_BTN_PERSON_B) {
                m_shm.SetIdentity(true);
                Logger::instance().Log(L"Switched to Person B");
            }

            if (wmId == ID_TOGGLE_WRITE) {
// ...
// ...
                bool checked = (SendMessage(GetDlgItem(m_hWnd, ID_TOGGLE_WRITE), BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (checked) {
                     m_shm.SetActiveWriter(m_shm.IsPersonB() ? 1 : 0);
                }
            }
// ...
// ...
// ...
// ...
// ...
            break;
        }
// ...
// ...
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}
// ...
// ...
// ...
// ... (Methods)
void Gui::OnNetStart() {
    wchar_t portBuf[10];
    GetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_PORT), portBuf, 10);
    int port = _wtoi(portBuf);

    m_network.Initialize();

    if (m_isServer) {
        m_network.StartServer(port);
    } else {
        wchar_t ipBuf[64];
        GetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_IP), ipBuf, 64);
        m_network.ConnectToServer(ipBuf, port);
    }
}

void Gui::OnNetSend() {
    wchar_t buf[256];
    GetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_SEND), buf, 256);
    if (wcslen(buf) == 0) return;
    
    // Echo locally
    std::wstring msg = L"Me: ";
    msg += buf;
    AppendText(GetDlgItem(m_hWnd, ID_LIST_CHAT), msg);
    
    // Send
    m_network.SendMessageTxt(buf);
    
    SetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_SEND), L"");
}

void Gui::OnShmSend() {
    wchar_t buf[256];
    GetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_SEND), buf, 256);
    if (wcslen(buf) == 0) return;
    
    // Format with name
    std::wstring fullMsg;
    if (m_shm.IsPersonB()) {
        fullMsg = m_shm.GetUserB();
    } else {
        fullMsg = m_shm.GetUserA();
    }
    fullMsg += L": ";
    fullMsg += buf;

    if (!m_shm.SendMessageTxt(fullMsg)) {
        // Failed to send, likely due to permission. Auto-take control.
        Logger::instance().Log(L"[SHM] Taking write control and sending...");
        m_shm.SetActiveWriter(m_shm.IsPersonB() ? 1 : 0);
        
        if (m_shm.SendMessageTxt(fullMsg)) {
             SetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_SEND), L"");
        } else {
             Logger::instance().Log(L"[SHM] Failed to send message.");
        }
    } else {
        SetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_SEND), L"");
    }
}

void Gui::Update() {
    // 1. Process Logs
    if (Logger::instance().HasNewLogs()) {
        auto logs = Logger::instance().GetLogs();
        HWND hLogList = GetDlgItem(m_hWnd, ID_LIST_LOGS);
        if (hLogList) {
            for (const auto& log : logs) {
                AppendText(hLogList, log);
            }
        }
    }

    // 2. Process Network Messages
    if (m_currentMode == MODE_SOCKET) {
        auto msgs = m_network.GetIncomingMessages();
        HWND hChat = GetDlgItem(m_hWnd, ID_LIST_CHAT);
        if (hChat) {
             for (const auto& msg : msgs) {
                 std::wstring display = L"Peer: ";
                 display += msg;
                 AppendText(hChat, display);
             }
        }
    }

    // 3. Process Shm State
    if (m_currentMode == MODE_SHM) {
        // Update names
        if (m_shm.IsPersonB()) {
            SetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_NAME_A), m_shm.GetUserA().c_str());
        } else {
            SetWindowTextW(GetDlgItem(m_hWnd, ID_EDIT_NAME_B), m_shm.GetUserB().c_str());
        }
        
        // Update Writer Toggle Checkbox state
        int active = m_shm.GetActiveWriter();
        HWND hToggle = GetDlgItem(m_hWnd, ID_TOGGLE_WRITE);
        bool shouldBeChecked = false;
        
        // Checkbox Logic: The checkbox means "Is [Current User] The Writer?"
        // Ideally, if I am Person A, and active is 0(A), then check it.
        // If I am Person B, and active is 1(B), then check it.
        
        if (!m_shm.IsPersonB() && active == 0) shouldBeChecked = true;
        if (m_shm.IsPersonB() && active == 1) shouldBeChecked = true;
        
        SendMessage(hToggle, BM_SETCHECK, shouldBeChecked ? BST_CHECKED : BST_UNCHECKED, 0);

        // Update Messages
        auto msgs = m_shm.GetMessages();
        if (!msgs.empty()) {
             HWND hChat = GetDlgItem(m_hWnd, ID_LIST_CHAT);
             // Smart update: if count differs, reload all (simple)
             // Or better: shared memory logic usually returns ALL messages for simplicity in this small project
             SendMessage(hChat, LB_RESETCONTENT, 0, 0);
             for (const auto& msg : msgs) {
                 AppendText(hChat, msg);
             }
        }
    }
}
