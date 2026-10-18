#include "ChalkApp.h"
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
        return window_->ProcessMessages([this]() {
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
