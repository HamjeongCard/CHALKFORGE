#include "ChalkError.h"

#include "ChalkLogger.h"

#include <cstdio>

std::string ChalkFormatHResult(HRESULT hr) {
    char* buffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD language = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

    const DWORD length =
        FormatMessageA(flags, nullptr, static_cast<DWORD>(hr), language,
                       reinterpret_cast<LPSTR>(&buffer), 0, nullptr);

    std::string message;
    if (length != 0 && buffer != nullptr) {
        message.assign(buffer, length);
        while (!message.empty() &&
               (message.back() == '\r' || message.back() == '\n' ||
                message.back() == ' ')) {
            message.pop_back();
        }
        LocalFree(buffer);
    }

    if (message.empty()) {
        message = "Unknown error";
    }

    char code[16];
    std::snprintf(code, sizeof(code), "0x%08lX",
                  static_cast<unsigned long>(hr));
    return std::string(code) + ": " + message;
}

namespace {

std::string BuildExceptionMessage(HRESULT hr, const char* expression,
                                  const char* file, int line) {
    char location[512];
    std::snprintf(location, sizeof(location), "%s(%d)", file != nullptr ? file : "?",
                  line);

    std::string message = "CHALKFORGE failure at ";
    message += location;
    message += ": ";
    message += expression != nullptr ? expression : "?";
    message += " -> ";
    message += ChalkFormatHResult(hr);
    return message;
}

} // namespace

ChalkException::ChalkException(HRESULT hr, const char* expression, const char* file,
                               int line)
    : std::runtime_error(BuildExceptionMessage(hr, expression, file, line)), hr_(hr) {
    ChalkLogger::Get().Error(what());
}
