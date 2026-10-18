#pragma once

#include <windows.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <vector>

class ChalkFence {
public:
    ChalkFence();
    ~ChalkFence();

    ChalkFence(const ChalkFence&) = delete;
    ChalkFence& operator=(const ChalkFence&) = delete;

    bool Initialize(ID3D12Device* device, UINT frameCount);
    void Shutdown();

    bool IsInitialized() const noexcept { return initialized_; }

    uint64_t Signal(ID3D12CommandQueue* queue, UINT frameIndex);
    bool WaitForValue(uint64_t value);
    bool WaitForFrame(UINT frameIndex);
    bool WaitForGpu(ID3D12CommandQueue* queue, UINT frameIndex);

    uint64_t CompletedValue() const noexcept;
    uint64_t FrameValue(UINT frameIndex) const noexcept;
    void SetFrameValue(UINT frameIndex, uint64_t value) noexcept;

private:
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    HANDLE event_ = nullptr;
    std::vector<uint64_t> frameValues_;
    UINT frameCount_ = 0;
    bool initialized_ = false;
};
