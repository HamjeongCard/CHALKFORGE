#pragma once

#include <cstdarg>
#include <fstream>
#include <mutex>
#include <string>

enum class ChalkLogLevel {
    Info,
    Warning,
    Error,
    Fatal
};

class ChalkLogger {
public:
    static ChalkLogger& Get();

    void Initialize(const char* logFilePath);
    void Shutdown();

    void Log(ChalkLogLevel level, const std::string& message);
    void Log(ChalkLogLevel level, const std::wstring& message);

    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(const std::string& message);
    void Fatal(const std::string& message);

    void InfoF(const char* format, ...);
    void WarningF(const char* format, ...);
    void ErrorF(const char* format, ...);
    void FatalF(const char* format, ...);

private:
    ChalkLogger() = default;
    ~ChalkLogger();
    ChalkLogger(const ChalkLogger&) = delete;
    ChalkLogger& operator=(const ChalkLogger&) = delete;

    void Write(ChalkLogLevel level, const std::string& message);
    void WriteFormatted(ChalkLogLevel level, const char* format, va_list args);

    std::ofstream file_;
    bool fileOpen_ = false;
    std::mutex mutex_;
};
