#include "ChalkWindow.h"

#include "ChalkConfig.h"

#include <windows.h>

ChalkWindow::ChalkWindow() = default;

ChalkWindow::~ChalkWindow() {
    if (hWnd_ != nullptr && IsWindow(hWnd_) != FALSE) {
        DestroyWindow(hWnd_);
    }
    hWnd_ = nullptr;
}

LRESULT CALLBACK ChalkWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    ChalkWindow* self = nullptr;

    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<ChalkWindow*>(create->lpCreateParams);
        self->hWnd_ = hWnd;
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<ChalkWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (self != nullptr) {
        return self->OnMessage(message, wParam, lParam);
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

LRESULT ChalkWindow::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            HandleResize(LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd_, &ps);
        FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
        EndPaint(hWnd_, &ps);
        return 0;
    }
    case WM_KEYDOWN:
        if (static_cast<std::size_t>(wParam) < keyDown_.size()) {
            keyDown_[static_cast<std::size_t>(wParam)] = true;
        }
        return 0;
    case WM_KEYUP:
        if (static_cast<std::size_t>(wParam) < keyDown_.size()) {
            keyDown_[static_cast<std::size_t>(wParam)] = false;
        }
        return 0;
    case WM_SYSKEYDOWN:
        if (static_cast<std::size_t>(wParam) < keyDown_.size()) {
            keyDown_[static_cast<std::size_t>(wParam)] = true;
        }
        return DefWindowProcW(hWnd_, message, wParam, lParam);
    case WM_SYSKEYUP:
        if (static_cast<std::size_t>(wParam) < keyDown_.size()) {
            keyDown_[static_cast<std::size_t>(wParam)] = false;
        }
        return DefWindowProcW(hWnd_, message, wParam, lParam);
    case WM_MOUSEMOVE: {
        const int x = static_cast<int>(static_cast<short>(LOWORD(lParam)));
        const int y = static_cast<int>(static_cast<short>(HIWORD(lParam)));
        if (mouseTracked_) {
            mouseDeltaX_ += x - mousePosition_.x;
            mouseDeltaY_ += y - mousePosition_.y;
        }
        mousePosition_.x = x;
        mousePosition_.y = y;
        mouseTracked_ = true;
        return 0;
    }
    case WM_LBUTTONDOWN:
        mouseDown_[kMouseLeft] = true;
        SetCapture(hWnd_);
        return 0;
    case WM_LBUTTONUP:
        mouseDown_[kMouseLeft] = false;
        ReleaseCapture();
        return 0;
    case WM_RBUTTONDOWN:
        mouseDown_[kMouseRight] = true;
        SetCapture(hWnd_);
        return 0;
    case WM_RBUTTONUP:
        mouseDown_[kMouseRight] = false;
        ReleaseCapture();
        return 0;
    case WM_MBUTTONDOWN:
        mouseDown_[kMouseMiddle] = true;
        SetCapture(hWnd_);
        return 0;
    case WM_MBUTTONUP:
        mouseDown_[kMouseMiddle] = false;
        ReleaseCapture();
        return 0;
    case WM_SETFOCUS:
        mouseTracked_ = false;
        return 0;
    case WM_KILLFOCUS:
        ResetInputState();
        return 0;
    default:
        return DefWindowProcW(hWnd_, message, wParam, lParam);
    }
}

void ChalkWindow::HandleResize(int width, int height) {
    width_ = width;
    height_ = height;

    if (resizeCallback_) {
        resizeCallback_(width, height);
    }
}

bool ChalkWindow::Create(HINSTANCE hInstance, int nCmdShow, LPCWSTR title) {
    nCmdShow_ = nCmdShow;

    WNDCLASSEXW wc = { };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &ChalkWindow::WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = ChalkConfig::kWindowClassName;

    if (RegisterClassExW(&wc) == 0) {
        return false;
    }

    RECT rect = { 0, 0, ChalkConfig::kDefaultWindowWidth, ChalkConfig::kDefaultWindowHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd_ = CreateWindowExW(
        0,
        ChalkConfig::kWindowClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (hWnd_ == nullptr) {
        return false;
    }

    width_ = ChalkConfig::kDefaultWindowWidth;
    height_ = ChalkConfig::kDefaultWindowHeight;

    ShowWindow(hWnd_, nCmdShow);
    UpdateWindow(hWnd_);

    return true;
}

int ChalkWindow::ProcessMessages(const std::function<void()>& onIdle) {
    MSG msg = { };

    for (;;) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return static_cast<int>(msg.wParam);
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (onIdle) {
            onIdle();
        }
    }
}

void ChalkWindow::SetResizeCallback(std::function<void(int, int)> callback) {
    resizeCallback_ = std::move(callback);
}

bool ChalkWindow::IsKeyDown(int virtualKey) const noexcept {
    if (virtualKey < 0 || static_cast<std::size_t>(virtualKey) >= keyDown_.size()) {
        return false;
    }
    return keyDown_[static_cast<std::size_t>(virtualKey)];
}

bool ChalkWindow::IsMouseButtonDown(int button) const noexcept {
    if (button < 0 || static_cast<std::size_t>(button) >= mouseDown_.size()) {
        return false;
    }
    return mouseDown_[static_cast<std::size_t>(button)];
}

void ChalkWindow::ConsumeMouseDelta(int& outDeltaX, int& outDeltaY) noexcept {
    outDeltaX = mouseDeltaX_;
    outDeltaY = mouseDeltaY_;
    mouseDeltaX_ = 0;
    mouseDeltaY_ = 0;
}

void ChalkWindow::ResetInputState() noexcept {
    keyDown_.fill(false);
    mouseDown_.fill(false);
    mouseDeltaX_ = 0;
    mouseDeltaY_ = 0;
    mouseTracked_ = false;
}
