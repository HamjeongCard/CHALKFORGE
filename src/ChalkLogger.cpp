#include "ChalkLogger.h"

#include <windows.h>

#include <cstdio>

namespace {

const char* LevelToString(ChalkLogLevel level) {
    switch (level) {
    case ChalkLogLevel::Info:
        return "INFO";
    case ChalkLogLevel::Warning:
        return "WARN";
    case ChalkLogLevel::Error:
        return "ERROR";
    case ChalkLogLevel::Fatal:
        return "FATAL";
    }
    return "UNKNOWN";
}

std::string CurrentTimestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buffer[40];
    std::snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u %02u:%02u:%02u.%03u",
                  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                  st.wMilliseconds);
    return std::string(buffer);
}

std::string WideToUtf8(const std::wstring& value) {
    if (value.empty()) {
        return std::string();
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(),
                                         static_cast<int>(value.size()), nullptr, 0,
                                         nullptr, nullptr);
    if (size <= 0) {
        return std::string();
    }
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
                        result.data(), size, nullptr, nullptr);
    return result;
}

} // namespace

ChalkLogger& ChalkLogger::Get() {
    static ChalkLogger instance;
    return instance;
}

ChalkLogger::~ChalkLogger() {
    Shutdown();
}

void ChalkLogger::Initialize(const char* logFilePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fileOpen_) {
        return;
    }
    file_.open(logFilePath, std::ios::out | std::ios::trunc);
    fileOpen_ = file_.is_open();
    if (!fileOpen_) {
        OutputDebugStringA("[CHALKFORGE][WARN] Failed to open log file\n");
    }
}

void ChalkLogger::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fileOpen_) {
        file_.flush();
        file_.close();
        fileOpen_ = false;
    }
}

void ChalkLogger::Log(ChalkLogLevel level, const std::string& message) {
    Write(level, message);
}

void ChalkLogger::Log(ChalkLogLevel level, const std::wstring& message) {
    Write(level, WideToUtf8(message));
}

void ChalkLogger::Info(const std::string& message) {
    Write(ChalkLogLevel::Info, message);
}

void ChalkLogger::Warning(const std::string& message) {
    Write(ChalkLogLevel::Warning, message);
}

void ChalkLogger::Error(const std::string& message) {
    Write(ChalkLogLevel::Error, message);
}

void ChalkLogger::Fatal(const std::string& message) {
    Write(ChalkLogLevel::Fatal, message);
}

void ChalkLogger::InfoF(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WriteFormatted(ChalkLogLevel::Info, format, args);
    va_end(args);
}

void ChalkLogger::WarningF(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WriteFormatted(ChalkLogLevel::Warning, format, args);
    va_end(args);
}

void ChalkLogger::ErrorF(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WriteFormatted(ChalkLogLevel::Error, format, args);
    va_end(args);
}

void ChalkLogger::FatalF(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WriteFormatted(ChalkLogLevel::Fatal, format, args);
    va_end(args);
}

void ChalkLogger::Write(ChalkLogLevel level, const std::string& message) {
    const std::string line = "[" + CurrentTimestamp() + "][" + LevelToString(level) +
                             "] " + message + "\n";

    std::lock_guard<std::mutex> lock(mutex_);

    OutputDebugStringA(line.c_str());

    if (fileOpen_) {
        file_ << line;
        file_.flush();
    }
}

void ChalkLogger::WriteFormatted(ChalkLogLevel level, const char* format,
                                 va_list args) {
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    Write(level, std::string(buffer));
}
