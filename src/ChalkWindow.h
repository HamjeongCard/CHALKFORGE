#pragma once

#include <windows.h>

#include <functional>

class ChalkWindow {
public:
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

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void HandleResize(int width, int height);

    HWND hWnd_ = nullptr;
    int nCmdShow_ = 0;
    int width_ = 0;
    int height_ = 0;
    std::function<void(int, int)> resizeCallback_;
};
