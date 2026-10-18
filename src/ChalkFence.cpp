#include "ChalkFence.h"

#include "ChalkError.h"
#include "ChalkLogger.h"

ChalkFence::ChalkFence() = default;

ChalkFence::~ChalkFence() {
    Shutdown();
}

bool ChalkFence::Initialize(ID3D12Device* device, UINT frameCount) {
    if (initialized_) {
        return true;
    }

    if (device == nullptr || frameCount == 0) {
        ChalkLogger::Get().Error("ChalkFence::Initialize received invalid arguments.");
        return false;
    }

    FAIL(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));

    frameCount_ = frameCount;
    frameValues_.assign(frameCount, 0);

    event_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (event_ == nullptr) {
        ChalkLogger::Get().Error("Failed to create fence event.");
        fence_.Reset();
        frameValues_.clear();
        frameCount_ = 0;
        return false;
    }

    initialized_ = true;
    return true;
}

void ChalkFence::Shutdown() {
    initialized_ = false;

    if (event_ != nullptr) {
        CloseHandle(event_);
        event_ = nullptr;
    }

    fence_.Reset();
    frameValues_.clear();
    frameCount_ = 0;
}

uint64_t ChalkFence::Signal(ID3D12CommandQueue* queue, UINT frameIndex) {
    if (!initialized_ || queue == nullptr || frameIndex >= frameCount_) {
        return 0;
    }

    const uint64_t value = frameValues_[frameIndex];
    FAIL(queue->Signal(fence_.Get(), value));
    return value;
}

bool ChalkFence::WaitForValue(uint64_t value) {
    if (!initialized_) {
        return false;
    }

    if (fence_->GetCompletedValue() >= value) {
        return true;
    }

    FAIL(fence_->SetEventOnCompletion(value, event_));
    return WaitForSingleObjectEx(event_, INFINITE, FALSE) == WAIT_OBJECT_0;
}

bool ChalkFence::WaitForFrame(UINT frameIndex) {
    if (!initialized_ || frameIndex >= frameCount_) {
        return false;
    }

    return WaitForValue(frameValues_[frameIndex]);
}

bool ChalkFence::WaitForGpu(ID3D12CommandQueue* queue, UINT frameIndex) {
    if (!initialized_ || queue == nullptr || frameIndex >= frameCount_) {
        return false;
    }

    const uint64_t value = frameValues_[frameIndex];
    FAIL(queue->Signal(fence_.Get(), value));

    const bool completed = WaitForValue(value);
    frameValues_[frameIndex] = value + 1;
    return completed;
}

uint64_t ChalkFence::CompletedValue() const noexcept {
    if (!initialized_) {
        return 0;
    }

    return fence_->GetCompletedValue();
}

uint64_t ChalkFence::FrameValue(UINT frameIndex) const noexcept {
    if (frameIndex >= frameCount_) {
        return 0;
    }

    return frameValues_[frameIndex];
}

void ChalkFence::SetFrameValue(UINT frameIndex, uint64_t value) noexcept {
    if (frameIndex >= frameCount_) {
        return;
    }

    frameValues_[frameIndex] = value;
}
