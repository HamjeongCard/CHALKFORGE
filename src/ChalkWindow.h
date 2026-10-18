#pragma once

#include <windows.h>

#include <array>
#include <cstddef>
#include <functional>

class ChalkWindow {
public:
    enum MouseButton : int {
        kMouseLeft = 0,
        kMouseRight = 1,
        kMouseMiddle = 2,
        kMouseButtonCount = 3
    };

    ChalkWindow();
    ~ChalkWindow();

    ChalkWindow(const ChalkWindow&) = delete;
    ChalkWindow& operator=(const ChalkWindow&) = delete;

    bool Create(HINSTANCE hInstance, int nCmdShow, LPCWSTR title);
    int ProcessMessages(const std::function<void()>& onIdle = { });
    void SetResizeCallback(std::function<void(int, int)> callback);

    HWND Handle() const noexcept { return hWnd_; }
    int Width() const noexcept { return width_; }
    int Height() const noexcept { return height_; }

    bool IsKeyDown(int virtualKey) const noexcept;
    bool IsMouseButtonDown(int button) const noexcept;
    POINT MousePosition() const noexcept { return mousePosition_; }
    void ConsumeMouseDelta(int& outDeltaX, int& outDeltaY) noexcept;

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void HandleResize(int width, int height);
    void ResetInputState() noexcept;

    HWND hWnd_ = nullptr;
    int nCmdShow_ = 0;
    int width_ = 0;
    int height_ = 0;
    std::function<void(int, int)> resizeCallback_;

    std::array<bool, 256> keyDown_{ };
    std::array<bool, kMouseButtonCount> mouseDown_{ };
    POINT mousePosition_{ 0, 0 };
    int mouseDeltaX_ = 0;
    int mouseDeltaY_ = 0;
    bool mouseTracked_ = false;
};
