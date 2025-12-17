#include "Network.h"
#include "Logger.h"
#include <codecvt>
#include <locale>

NetworkManager::NetworkManager() : m_running(false), m_connected(false) {}

NetworkManager::~NetworkManager() {
    Shutdown();
}

bool NetworkManager::Initialize() {
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        Logger::instance().Log(L"[Net] WSAStartup failed.");
        return false;
    }
    return true;
}

void NetworkManager::Shutdown() {
    m_running = false;
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    if (m_receiveThread.joinable()) {
        m_receiveThread.join();
    }
    m_connected = false;
    WSACleanup();
}

bool NetworkManager::StartServer(int port) {
    if (m_connected) Shutdown();
    m_running = true;

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        Logger::instance().Log(L"[Server] Error at socket().");
        return false;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);

    if (bind(listenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        Logger::instance().Log(L"[Server] bind() failed.");
        closesocket(listenSocket);
        return false;
    }

    if (listen(listenSocket, 1) == SOCKET_ERROR) {
        Logger::instance().Log(L"[Server] listen() failed.");
        closesocket(listenSocket);
        return false;
    }

    Logger::instance().Log(L"[Server] Waiting for client connection...");

    // Accept in a separate thread to not block UI? 
    // For simplicity, we'll do it in a thread, but we treat the first connection as THE connection.
    m_receiveThread = std::thread([this, listenSocket]() {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
             Logger::instance().Log(L"[Server] accept failed.");
             closesocket(listenSocket);
             return;
        }
        
        closesocket(listenSocket); // Stop listening after one client
        m_socket = clientSocket;
        m_connected = true;
        Logger::instance().Log(L"[Server] Client connected!");

        ReceiveLoop();
    });

    return true;
}

bool NetworkManager::ConnectToServer(const std::wstring& ip, int port) {
    if (m_connected) Shutdown();
    m_running = true;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        Logger::instance().Log(L"[Client] Error at socket().");
        return false;
    }

    // Convert wstring IP to string
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string ipStr = converter.to_bytes(ip);

    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(ipStr.c_str());
    clientService.sin_port = htons(port);

    Logger::instance().Log(L"[Client] Connecting...");

    if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
        Logger::instance().Log(L"[Client] Failed to connect.");
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    m_connected = true;
    Logger::instance().Log(L"[Client] Connected to server!");

    m_receiveThread = std::thread(&NetworkManager::ReceiveLoop, this);
    return true;
}

void NetworkManager::SendMessageTxt(const std::wstring& msg) {
    if (!m_connected || m_socket == INVALID_SOCKET) return;

    // Send UTF-8
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string sendBuf = converter.to_bytes(msg);

    int result = send(m_socket, sendBuf.c_str(), (int)sendBuf.length(), 0);
    if (result == SOCKET_ERROR) {
         Logger::instance().Log(L"[Net] Send failed.");
         m_connected = false;
         closesocket(m_socket);
    }
}

void NetworkManager::ReceiveLoop() {
    char recvbuf[512];
    int recvbuflen = 512;

    while (m_running && m_connected) {
        int iResult = recv(m_socket, recvbuf, recvbuflen - 1, 0);
        if (iResult > 0) {
            recvbuf[iResult] = '\0';
            
            // Convert back to wstring
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            try {
                std::wstring wMsg = converter.from_bytes(recvbuf);
                
                std::lock_guard<std::mutex> lock(m_msgMutex);
                m_incomingMessages.push_back(wMsg);
            } catch (...) {
                 // Conversion error
            }

        } else if (iResult == 0) {
            Logger::instance().Log(L"[Net] Connection closed.");
            m_connected = false;
            break;
        } else {
             Logger::instance().Log(L"[Net] recv failed.");
             m_connected = false;
             break;
        }
    }
}

std::vector<std::wstring> NetworkManager::GetIncomingMessages() {
    std::lock_guard<std::mutex> lock(m_msgMutex);
    std::vector<std::wstring> msgs = m_incomingMessages;
    m_incomingMessages.clear();
    return msgs;
}

bool NetworkManager::IsConnected() const {
    return m_connected;
}
