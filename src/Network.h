#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool Initialize(); // WSAStartup
    void Shutdown();   // WSACleanup, close sockets

    // Server
    bool StartServer(int port);
    
    // Client
    bool ConnectToServer(const std::wstring& ip, int port);

    // Common
    void SendMessageTxt(const std::wstring& msg);
    std::vector<std::wstring> GetIncomingMessages();
    
    bool IsConnected() const;

private:
    void ReceiveLoop();

    SOCKET m_socket = INVALID_SOCKET;
    std::atomic<bool> m_running;
    std::atomic<bool> m_connected;
    std::thread m_receiveThread;

    std::vector<std::wstring> m_incomingMessages;
    std::mutex m_msgMutex;
};
