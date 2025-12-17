#pragma once
#include <windows.h>
#include <string>
#include <functional>

// Shared Memory Data Structure
struct ShmChatData {
    wchar_t userA[32];
    wchar_t userB[32];
    wchar_t messages[50][256]; // Last 50 messages
    int msgCount;
    int activeWriter; // 0 = Person A, 1 = Person B
    long msgVersion;  // Incremented on new message
};

class SharedMemoryManager {
public:
    SharedMemoryManager();
    ~SharedMemoryManager();

    bool Initialize();
    void Cleanup();

    // Actions
    void SetIdentity(bool isPersonB);
    void SetName(const std::wstring& name);
    bool SendMessageTxt(const std::wstring& msg);
    void SetActiveWriter(int role); // 0 or 1

    // State Readers
    std::wstring GetUserA();
    std::wstring GetUserB();
    int GetActiveWriter();
    std::vector<std::wstring> GetMessages();

    bool IsPersonB() const { return m_isPersonB; }

private:
    HANDLE m_hMapFile = NULL;
    ShmChatData* m_pData = NULL;
    HANDLE m_hMutex = NULL; // For write access safety

    bool m_isPersonB = false;
    long m_lastSeenVersion = 0;
};
