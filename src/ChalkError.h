#pragma once

#include <windows.h>

#include <stdexcept>
#include <string>

std::string ChalkFormatHResult(HRESULT hr);

class ChalkException : public std::runtime_error {
public:
    ChalkException(HRESULT hr, const char* expression, const char* file, int line);

    HRESULT Code() const noexcept { return hr_; }

private:
    HRESULT hr_;
};

#define FAIL(expression)                                                          \
    do {                                                                          \
        HRESULT chalk_hresult_ = (expression);                                    \
        if (FAILED(chalk_hresult_)) {                                             \
            throw ChalkException(chalk_hresult_, #expression, __FILE__, __LINE__); \
        }                                                                         \
    } while (0)

#define THROW_IF_FAILED(expression) FAIL(expression)
