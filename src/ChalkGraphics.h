#pragma once

#include "ChalkConfig.h"

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdint>

class ChalkGraphics {
public:
    ChalkGraphics();
    ~ChalkGraphics();

    ChalkGraphics(const ChalkGraphics&) = delete;
    ChalkGraphics& operator=(const ChalkGraphics&) = delete;

    bool Initialize(HWND hWnd, int width, int height);
    void Finalize();

    void RenderFrame();
    void OnResize(int width, int height);
    void WaitForGpu();

    bool IsInitialized() const noexcept { return initialized_; }

private:
    static constexpr UINT kFrameCount = static_cast<UINT>(ChalkConfig::kFrameCount);

    bool CreateDeviceAndQueue();
    bool CreateSwapChain(HWND hWnd, int width, int height);
    bool CreateDescriptorHeaps();
    bool CreateRenderTargets();
    bool CreateCommandObjects();
    bool CreateFence();
    bool CreatePipeline();
    bool CreateTriangleGeometry();

    void PopulateCommandList();
    void MoveToNextFrame();
    void ReleaseResources();

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
    Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets_[kFrameCount];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators_[kFrameCount];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = { };

    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    HANDLE fenceEvent_ = nullptr;
    UINT64 frameFenceValues_[kFrameCount] = { };

    UINT rtvDescriptorSize_ = 0;
    UINT frameIndex_ = 0;
    int width_ = 0;
    int height_ = 0;
    bool initialized_ = false;
};
