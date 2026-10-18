#pragma once

#include "ChalkCamera.h"
#include "ChalkConfig.h"
#include "ChalkFence.h"
#include "ChalkMaterial.h"
#include "ChalkMesh.h"
#include "ChalkResource.h"
#include "ChalkScene.h"

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

    ChalkCamera& Camera() noexcept { return camera_; }
    const ChalkCamera& Camera() const noexcept { return camera_; }

private:
    static constexpr UINT kFrameCount = static_cast<UINT>(ChalkConfig::kFrameCount);
    static constexpr UINT kMaterialCount = 2;
    static constexpr UINT kMaxSceneObjects = 16;

    bool CreateDeviceAndQueue();
    bool CreateSwapChain(HWND hWnd, int width, int height);
    bool CreateDescriptorHeaps();
    bool CreateRenderTargets();
    bool CreateDepthStencilBuffer(int width, int height);
    bool CreateCommandObjects();
    bool CreateFence();
    bool CreatePipeline();
    bool CreateSceneResources();
    bool CreateUploadBuffer(UINT64 size, const wchar_t* name,
                            Microsoft::WRL::ComPtr<ID3D12Resource>& outResource);

    void PopulateCommandList();
    void MoveToNextFrame();
    void ReleaseResources();

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
    Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets_[kFrameCount];
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
    ChalkResource depthStencils_[kFrameCount];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators_[kFrameCount];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    Microsoft::WRL::ComPtr<ID3D12Resource> objectConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> lightConstantBuffer_;

    ChalkCamera camera_;
    ChalkScene scene_;
    ChalkMesh cubeMesh_;
    ChalkMaterial materials_[kMaterialCount];

    ChalkFence fence_;

    UINT rtvDescriptorSize_ = 0;
    UINT dsvDescriptorSize_ = 0;
    UINT frameIndex_ = 0;
    int width_ = 0;
    int height_ = 0;
    bool initialized_ = false;
};
