#include "SharedMemory.h"
#include "Logger.h"
#include <vector>

const wchar_t* SHM_NAME = L"Local\\MyChatSuiteShm";
const wchar_t* MUTEX_NAME = L"Local\\MyChatSuiteMutex";

SharedMemoryManager::SharedMemoryManager() {}

SharedMemoryManager::~SharedMemoryManager() {
    Cleanup();
}

void SharedMemoryManager::Cleanup() {
    if (m_pData) {
        UnmapViewOfFile(m_pData);
        m_pData = NULL;
    }
    if (m_hMapFile) {
        CloseHandle(m_hMapFile);
        m_hMapFile = NULL;
    }
    if (m_hMutex) {
        CloseHandle(m_hMutex);
        m_hMutex = NULL;
    }
}

void SharedMemoryManager::SetIdentity(bool isPersonB) {
    m_isPersonB = isPersonB;
}

bool SharedMemoryManager::Initialize() {
    // default to Person A if not set, but member var preserves state if set before? 
    // Actually we just init valid handles. Identity is separate.
    
    if (m_hMapFile) return true; // Already initialized

    // Create/Open Mutex
    m_hMutex = CreateMutexW(NULL, FALSE, MUTEX_NAME);
    if (!m_hMutex) {
        Logger::instance().Log(L"[SHM] Failed to create mutex.");
        return false;
    }

    // Create/Open File Mapping
    m_hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,    // Use paging file
        NULL,                    // Default security
        PAGE_READWRITE,          // Read/write access
        0,                       // Max. object size (high-order)
        sizeof(ShmChatData),     // Max. object size (low-order)
        SHM_NAME                 // Name of mapping object
    );

    if (m_hMapFile == NULL) {
        std::wstring err = L"[SHM] Could not create file mapping object. Error: " + std::to_wstring(GetLastError());
        Logger::instance().Log(err);
        return false;
    }

    bool alreadyExists = (GetLastError() == ERROR_ALREADY_EXISTS);

    m_pData = (ShmChatData*)MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ShmChatData));
    if (m_pData == NULL) {
        Logger::instance().Log(L"[SHM] Could not map view of file.");
        CloseHandle(m_hMapFile);
        return false;
    }

    if (!alreadyExists) {
        // Initialize data if we are the first creator
        WaitForSingleObject(m_hMutex, INFINITE);
        memset(m_pData, 0, sizeof(ShmChatData));
        wcscpy_s(m_pData->userA, L"User A");
        wcscpy_s(m_pData->userB, L"User B");
        m_pData->activeWriter = 0; // Default A
        ReleaseMutex(m_hMutex);
        Logger::instance().Log(L"[SHM] Shared memory initialized.");
    } else {
        Logger::instance().Log(L"[SHM] Connected to existing shared memory.");
    }

    return true;
}

void SharedMemoryManager::SetName(const std::wstring& name) {
    if (!m_pData) return;
    WaitForSingleObject(m_hMutex, INFINITE);
    if (m_isPersonB) {
        wcsncpy_s(m_pData->userB, name.c_str(), 31);
    } else {
        wcsncpy_s(m_pData->userA, name.c_str(), 31);
    }
    ReleaseMutex(m_hMutex);
}

void SharedMemoryManager::SetActiveWriter(int role) {
    if (!m_pData) return;
    WaitForSingleObject(m_hMutex, INFINITE);
    m_pData->activeWriter = role;
    ReleaseMutex(m_hMutex);
}

bool SharedMemoryManager::SendMessageTxt(const std::wstring& msg) {
    if (!m_pData) return false;
    
    WaitForSingleObject(m_hMutex, INFINITE);
    
    // Check permission
    if ((m_isPersonB && m_pData->activeWriter != 1) || (!m_isPersonB && m_pData->activeWriter != 0)) {
        ReleaseMutex(m_hMutex);
        Logger::instance().Log(L"[SHM] Not your turn to write!");
        return false;
    }

    if (m_pData->msgCount < 50) {
        wcsncpy_s(m_pData->messages[m_pData->msgCount], msg.c_str(), 255);
        m_pData->msgCount++;
        m_pData->msgVersion++;
    } else {
        // Shift buffer (simple implementation for ring buffer behavior)
        for (int i = 0; i < 49; i++) {
            wcscpy_s(m_pData->messages[i], m_pData->messages[i+1]);
        }
        wcsncpy_s(m_pData->messages[49], msg.c_str(), 255);
        m_pData->msgVersion++;
    }

    ReleaseMutex(m_hMutex);
    return true;
}

std::wstring SharedMemoryManager::GetUserA() {
    if (!m_pData) return L"";
    // Optimistic read
    return m_pData->userA;
}

std::wstring SharedMemoryManager::GetUserB() {
    if (!m_pData) return L"";
    return m_pData->userB;
}

int SharedMemoryManager::GetActiveWriter() {
    if (!m_pData) return 0;
    return m_pData->activeWriter;
}

std::vector<std::wstring> SharedMemoryManager::GetMessages() {
    if (!m_pData) return {};
    
    std::vector<std::wstring> msgs;
    WaitForSingleObject(m_hMutex, INFINITE);
    if (m_lastSeenVersion != m_pData->msgVersion) {
        for (int i = 0; i < m_pData->msgCount; i++) {
            msgs.push_back(m_pData->messages[i]);
        }
        m_lastSeenVersion = m_pData->msgVersion;
    }
    ReleaseMutex(m_hMutex);
    return msgs; 
}
