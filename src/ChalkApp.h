#pragma once

#include <windows.h>

#include <memory>

class ChalkWindow;
class ChalkGraphics;

class ChalkApplication {
public:
    ChalkApplication();
    ~ChalkApplication();

    ChalkApplication(const ChalkApplication&) = delete;
    ChalkApplication& operator=(const ChalkApplication&) = delete;

    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    int Run();
    void Finalize();

    void OnResize(int width, int height);

    HINSTANCE Instance() const noexcept { return hInstance_; }
    int CmdShow() const noexcept { return nCmdShow_; }

private:
    float ComputeDeltaSeconds();
    void UpdateCamera(float deltaSeconds);

    HINSTANCE hInstance_ = nullptr;
    int nCmdShow_ = 0;
    bool initialized_ = false;

    int width_ = 0;
    int height_ = 0;

    LARGE_INTEGER performanceFrequency_{ };
    LARGE_INTEGER lastFrameCounter_{ };
    bool hasLastFrameCounter_ = false;

    std::unique_ptr<ChalkWindow> window_;
    std::unique_ptr<ChalkGraphics> graphics_;
};
