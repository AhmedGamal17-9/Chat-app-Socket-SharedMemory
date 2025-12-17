#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <deque>

// Simple thread-safe logger to store messages for the GUI
class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void Log(const std::wstring& msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logs.push_back(msg);
        if (m_logs.size() > 100) m_logs.pop_front();
        m_hasNewLogs = true;
    }

    bool HasNewLogs() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_hasNewLogs;
    }

    std::vector<std::wstring> GetLogs() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_hasNewLogs = false;
        return {m_logs.begin(), m_logs.end()};
    }

private:
    std::deque<std::wstring> m_logs;
    bool m_hasNewLogs = false;
    std::mutex m_mutex;
};
