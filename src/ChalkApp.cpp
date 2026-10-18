#include "ChalkApp.h"
#include "ChalkCamera.h"
#include "ChalkConfig.h"
#include "ChalkError.h"
#include "ChalkGraphics.h"
#include "ChalkLogger.h"
#include "ChalkWindow.h"

#include <windows.h>

#include <exception>

ChalkApplication::ChalkApplication() = default;
ChalkApplication::~ChalkApplication() = default;

bool ChalkApplication::Initialize(HINSTANCE hInstance, int nCmdShow) {
    if (initialized_) {
        return true;
    }

    hInstance_ = hInstance;
    nCmdShow_ = nCmdShow;

    ChalkLogger::Get().Initialize(ChalkConfig::kLogFileName);
    ChalkLogger::Get().Info("CHALKFORGE initializing");

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    window_ = std::make_unique<ChalkWindow>();
    if (!window_->Create(hInstance, nCmdShow, ChalkConfig::kWindowTitle)) {
        ChalkLogger::Get().Error("Failed to create window");
        window_.reset();
        return false;
    }

    window_->SetResizeCallback([this](int width, int height) {
        OnResize(width, height);
    });

    width_ = window_->Width();
    height_ = window_->Height();

    graphics_ = std::make_unique<ChalkGraphics>();
    if (!graphics_->Initialize(window_->Handle(), width_, height_)) {
        ChalkLogger::Get().Error("Failed to initialize graphics");
        graphics_.reset();
        window_.reset();
        return false;
    }

    initialized_ = true;
    return true;
}

int ChalkApplication::Run() {
    if (!initialized_) {
        ChalkLogger::Get().Error("ChalkApplication::Run called before Initialize");
        return 1;
    }

    try {
        QueryPerformanceFrequency(&performanceFrequency_);
        QueryPerformanceCounter(&lastFrameCounter_);
        hasLastFrameCounter_ = true;

        return window_->ProcessMessages([this]() {
            const float deltaSeconds = ComputeDeltaSeconds();
            UpdateCamera(deltaSeconds);
            graphics_->RenderFrame();
        });
    } catch (const std::exception& e) {
        ChalkLogger::Get().ErrorF("Unhandled exception in render loop: %s", e.what());
        return 1;
    }
}

void ChalkApplication::Finalize() {
    if (!initialized_) {
        return;
    }

    if (graphics_) {
        graphics_->Finalize();
        graphics_.reset();
    }

    window_.reset();

    ChalkLogger::Get().Info("CHALKFORGE shutting down");
    ChalkLogger::Get().Shutdown();

    initialized_ = false;
}

float ChalkApplication::ComputeDeltaSeconds() {
    LARGE_INTEGER now = { };
    QueryPerformanceCounter(&now);

    if (!hasLastFrameCounter_ || performanceFrequency_.QuadPart <= 0) {
        lastFrameCounter_ = now;
        hasLastFrameCounter_ = true;
        return 0.0f;
    }

    const double frequency = static_cast<double>(performanceFrequency_.QuadPart);
    double deltaSeconds = static_cast<double>(now.QuadPart - lastFrameCounter_.QuadPart) / frequency;
    lastFrameCounter_ = now;

    if (deltaSeconds < 0.0) {
        deltaSeconds = 0.0;
    }

    const double maxDeltaSeconds = static_cast<double>(ChalkConfig::kMaxFrameDeltaSeconds);
    if (deltaSeconds > maxDeltaSeconds) {
        deltaSeconds = maxDeltaSeconds;
    }

    return static_cast<float>(deltaSeconds);
}

void ChalkApplication::UpdateCamera(float deltaSeconds) {
    if (!window_ || !graphics_) {
        return;
    }

    const float moveDistance = ChalkConfig::kCameraMoveSpeed * deltaSeconds;

    float forward = 0.0f;
    float strafe = 0.0f;

    if (window_->IsKeyDown('W')) {
        forward += 1.0f;
    }
    if (window_->IsKeyDown('S')) {
        forward -= 1.0f;
    }
    if (window_->IsKeyDown('D')) {
        strafe += 1.0f;
    }
    if (window_->IsKeyDown('A')) {
        strafe -= 1.0f;
    }

    ChalkCamera& camera = graphics_->Camera();

    if (forward != 0.0f) {
        camera.MoveForward(forward * moveDistance);
    }
    if (strafe != 0.0f) {
        camera.MoveRight(strafe * moveDistance);
    }

    int deltaX = 0;
    int deltaY = 0;
    window_->ConsumeMouseDelta(deltaX, deltaY);

    if (window_->IsMouseButtonDown(ChalkWindow::kMouseLeft) && (deltaX != 0 || deltaY != 0)) {
        const float yaw = -static_cast<float>(deltaX) * ChalkConfig::kCameraOrbitSensitivity;
        const float pitch = -static_cast<float>(deltaY) * ChalkConfig::kCameraOrbitSensitivity;
        camera.Orbit(yaw, pitch);
    }
}

void ChalkApplication::OnResize(int width, int height) {
    width_ = width;
    height_ = height;

    if (graphics_ && graphics_->IsInitialized() && width > 0 && height > 0) {
        graphics_->OnResize(width, height);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    ChalkApplication app;
    if (!app.Initialize(hInstance, nCmdShow)) {
        return 1;
    }

    int exitCode = app.Run();
    app.Finalize();

    return exitCode;
}
