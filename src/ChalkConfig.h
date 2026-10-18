#pragma once

struct ChalkConfig {
    static constexpr wchar_t kWindowClassName[] = L"ChalkforgeWindowClass";
    static constexpr wchar_t kWindowTitle[] = L"CHALKFORGE";

    static constexpr int kDefaultWindowWidth = 1280;
    static constexpr int kDefaultWindowHeight = 720;

    static constexpr int kSwapChainBufferCount = 3;
    static constexpr int kFrameCount = kSwapChainBufferCount;

    static constexpr const char* kLogFileName = "chalkforge.log";
    static constexpr bool kEnableFileLogging = true;
};
