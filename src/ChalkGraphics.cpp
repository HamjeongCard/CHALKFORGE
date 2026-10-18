#include "ChalkGraphics.h"

#include "ChalkError.h"
#include "ChalkLogger.h"

#include <d3dcompiler.h>

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace {

const D3D_FEATURE_LEVEL kFeatureLevels[] = {
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
};

const float kClearColor[4] = { 0.05f, 0.05f, 0.08f, 1.0f };

struct Vertex {
    float position[3];
    float color[4];
};

std::string WideToUtf8(const std::wstring& text) {
    if (text.empty()) {
        return std::string();
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(),
                                         static_cast<int>(text.size()), nullptr, 0,
                                         nullptr, nullptr);
    if (size <= 0) {
        return std::string();
    }
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                        result.data(), size, nullptr, nullptr);
    return result;
}

std::wstring GetShaderDirectory() {
    wchar_t buffer[MAX_PATH] = { };
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    std::wstring path(buffer, length);
    const size_t slash = path.find_last_of(L"\\/");
    if (slash != std::wstring::npos) {
        path.erase(slash + 1);
    }
    path += L"shaders\\";
    return path;
}

bool ReadFileBytes(const std::wstring& path, std::vector<char>& out) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }
    const std::streamsize size = file.tellg();
    if (size <= 0) {
        return false;
    }
    file.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(size));
    file.read(out.data(), size);
    return file.good() || file.eof();
}

Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& path,
                                               const char* entryPoint,
                                               const char* target) {
    std::vector<char> source;
    if (!ReadFileBytes(path, source)) {
        const std::string narrow = WideToUtf8(path);
        ChalkLogger::Get().ErrorF("Failed to read shader file: %s", narrow.c_str());
        return nullptr;
    }

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef CHALK_DEBUG_LAYER
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    const std::string narrowPath = WideToUtf8(path);
    Microsoft::WRL::ComPtr<ID3DBlob> code;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    const HRESULT hr = D3DCompile(source.data(), source.size(), narrowPath.c_str(),
                                  nullptr, nullptr, entryPoint, target, flags, 0,
                                  &code, &errors);
    if (FAILED(hr)) {
        if (errors) {
            ChalkLogger::Get().ErrorF("Shader compile error in %s: %s", narrowPath.c_str(),
                                      static_cast<const char*>(errors->GetBufferPointer()));
        } else {
            ChalkLogger::Get().ErrorF("Shader compile failed in %s (%s)",
                                      narrowPath.c_str(), ChalkFormatHResult(hr).c_str());
        }
        return nullptr;
    }
    return code;
}

} // namespace

ChalkGraphics::ChalkGraphics() = default;

ChalkGraphics::~ChalkGraphics() {
    Finalize();
}

bool ChalkGraphics::Initialize(HWND hWnd, int width, int height) {
    if (initialized_) {
        return true;
    }

    try {
#ifdef CHALK_DEBUG_LAYER
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
            ChalkLogger::Get().Info("D3D12 debug layer enabled.");
        }
#endif

        if (!CreateDeviceAndQueue()) {
            return false;
        }
        if (!CreateSwapChain(hWnd, width, height)) {
            return false;
        }
        if (!CreateDescriptorHeaps()) {
            return false;
        }
        if (!CreateRenderTargets()) {
            return false;
        }
        if (!CreateCommandObjects()) {
            return false;
        }
        if (!CreateFence()) {
            return false;
        }
        if (!CreatePipeline()) {
            return false;
        }
        if (!CreateTriangleGeometry()) {
            return false;
        }

        width_ = width;
        height_ = height;
    } catch (const std::exception& e) {
        ChalkLogger::Get().ErrorF("Graphics initialization failed: %s", e.what());
        ReleaseResources();
        return false;
    }

    initialized_ = true;
    ChalkLogger::Get().InfoF("D3D12 graphics initialized (%dx%d).", width, height);
    return true;
}

bool ChalkGraphics::CreateDeviceAndQueue() {
    FAIL(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory_)));

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; factory_->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc = { };
        adapter->GetDesc1(&desc);
        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) {
            adapter.Reset();
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                        __uuidof(ID3D12Device), nullptr))) {
            break;
        }
        adapter.Reset();
    }

    HRESULT hr = E_FAIL;
    for (D3D_FEATURE_LEVEL level : kFeatureLevels) {
        hr = D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(&device_));
        if (SUCCEEDED(hr)) {
            break;
        }
    }
    FAIL(hr);

    D3D12_COMMAND_QUEUE_DESC queueDesc = { };
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;
    FAIL(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_)));

    return true;
}

bool ChalkGraphics::CreateSwapChain(HWND hWnd, int width, int height) {
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
    swapChainDesc.Width = static_cast<UINT>(width);
    swapChainDesc.Height = static_cast<UINT>(height);
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = kFrameCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    FAIL(factory_->CreateSwapChainForHwnd(commandQueue_.Get(), hWnd, &swapChainDesc,
                                          nullptr, nullptr, &swapChain));
    FAIL(factory_->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    FAIL(swapChain.As(&swapChain_));

    frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
    return true;
}

bool ChalkGraphics::CreateDescriptorHeaps() {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = { };
    rtvHeapDesc.NumDescriptors = kFrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    FAIL(device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_)));

    rtvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    return true;
}

bool ChalkGraphics::CreateRenderTargets() {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < kFrameCount; ++i) {
        FAIL(swapChain_->GetBuffer(i, IID_PPV_ARGS(&renderTargets_[i])));
        device_->CreateRenderTargetView(renderTargets_[i].Get(), nullptr, handle);
        handle.ptr += rtvDescriptorSize_;
    }
    return true;
}

bool ChalkGraphics::CreateCommandObjects() {
    for (UINT i = 0; i < kFrameCount; ++i) {
        FAIL(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                             IID_PPV_ARGS(&commandAllocators_[i])));
    }
    FAIL(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                    commandAllocators_[0].Get(), nullptr,
                                    IID_PPV_ARGS(&commandList_)));
    FAIL(commandList_->Close());
    return true;
}

bool ChalkGraphics::CreateFence() {
    FAIL(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
    for (UINT i = 0; i < kFrameCount; ++i) {
        frameFenceValues_[i] = 0;
    }
    fenceEvent_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent_ == nullptr) {
        ChalkLogger::Get().Error("Failed to create fence event.");
        return false;
    }
    return true;
}

bool ChalkGraphics::CreatePipeline() {
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = { };
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> signatureError;
    FAIL(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                     &signature, &signatureError));
    FAIL(device_->CreateRootSignature(0, signature->GetBufferPointer(),
                                      signature->GetBufferSize(),
                                      IID_PPV_ARGS(&rootSignature_)));

    const std::wstring shaderDir = GetShaderDirectory();
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader = CompileShader(shaderDir + L"TriangleVS.hlsl", "main", "vs_5_0");
    if (!vertexShader) {
        return false;
    }
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader = CompileShader(shaderDir + L"TrianglePS.hlsl", "main", "ps_5_0");
    if (!pixelShader) {
        return false;
    }

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position),
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color),
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_RASTERIZER_DESC rasterizerState = { };
    rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerState.FrontCounterClockwise = FALSE;
    rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerState.DepthClipEnable = TRUE;
    rasterizerState.MultisampleEnable = FALSE;
    rasterizerState.AntialiasedLineEnable = FALSE;
    rasterizerState.ForcedSampleCount = 0;
    rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_BLEND_DESC blendState = { };
    blendState.AlphaToCoverageEnable = FALSE;
    blendState.IndependentBlendEnable = FALSE;
    blendState.RenderTarget[0].BlendEnable = FALSE;
    blendState.RenderTarget[0].LogicOpEnable = FALSE;
    blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    blendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = { };
    pipelineDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineDesc.pRootSignature = rootSignature_.Get();
    pipelineDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    pipelineDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    pipelineDesc.RasterizerState = rasterizerState;
    pipelineDesc.BlendState = blendState;
    pipelineDesc.DepthStencilState.DepthEnable = FALSE;
    pipelineDesc.DepthStencilState.StencilEnable = FALSE;
    pipelineDesc.SampleMask = UINT_MAX;
    pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.NumRenderTargets = 1;
    pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineDesc.SampleDesc.Count = 1;
    FAIL(device_->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState_)));

    return true;
}

bool ChalkGraphics::CreateTriangleGeometry() {
    const Vertex vertices[] = {
        { { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
    };
    const UINT bufferSize = sizeof(vertices);

    D3D12_HEAP_PROPERTIES heapProps = { };
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC bufferDesc = { };
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = bufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    FAIL(device_->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                          IID_PPV_ARGS(&vertexBuffer_)));

    void* mapped = nullptr;
    const D3D12_RANGE readRange = { 0, 0 };
    FAIL(vertexBuffer_->Map(0, &readRange, &mapped));
    memcpy(mapped, vertices, bufferSize);
    vertexBuffer_->Unmap(0, nullptr);

    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.StrideInBytes = sizeof(Vertex);
    vertexBufferView_.SizeInBytes = bufferSize;
    return true;
}

void ChalkGraphics::RenderFrame() {
    if (!initialized_) {
        return;
    }

    FAIL(commandAllocators_[frameIndex_]->Reset());
    FAIL(commandList_->Reset(commandAllocators_[frameIndex_].Get(), pipelineState_.Get()));

    PopulateCommandList();

    ID3D12CommandList* commandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, commandLists);

    FAIL(swapChain_->Present(1, 0));

    MoveToNextFrame();
}

void ChalkGraphics::PopulateCommandList() {
    D3D12_RESOURCE_BARRIER barrier = { };
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = renderTargets_[frameIndex_].Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList_->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(frameIndex_) * rtvDescriptorSize_;

    commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    commandList_->ClearRenderTargetView(rtvHandle, kClearColor, 0, nullptr);

    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetPipelineState(pipelineState_.Get());
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D12_VIEWPORT viewport = { };
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width_);
    viewport.Height = static_cast<float>(height_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    commandList_->RSSetViewports(1, &viewport);

    const D3D12_RECT scissorRect = { 0, 0, width_, height_ };
    commandList_->RSSetScissorRects(1, &scissorRect);

    commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList_->DrawInstanced(3, 1, 0, 0);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList_->ResourceBarrier(1, &barrier);

    FAIL(commandList_->Close());
}

void ChalkGraphics::MoveToNextFrame() {
    const UINT64 currentFenceValue = frameFenceValues_[frameIndex_];
    FAIL(commandQueue_->Signal(fence_.Get(), currentFenceValue));

    frameIndex_ = swapChain_->GetCurrentBackBufferIndex();

    if (fence_->GetCompletedValue() < frameFenceValues_[frameIndex_]) {
        FAIL(fence_->SetEventOnCompletion(frameFenceValues_[frameIndex_], fenceEvent_));
        WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);
    }

    frameFenceValues_[frameIndex_] = currentFenceValue + 1;
}

void ChalkGraphics::WaitForGpu() {
    if (!commandQueue_ || !fence_ || !fenceEvent_) {
        return;
    }

    FAIL(commandQueue_->Signal(fence_.Get(), frameFenceValues_[frameIndex_]));
    if (fence_->GetCompletedValue() < frameFenceValues_[frameIndex_]) {
        FAIL(fence_->SetEventOnCompletion(frameFenceValues_[frameIndex_], fenceEvent_));
        WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);
    }
    frameFenceValues_[frameIndex_]++;
}

void ChalkGraphics::OnResize(int width, int height) {
    if (!initialized_) {
        return;
    }
    if (width == 0 || height == 0) {
        return;
    }
    if (width == width_ && height == height_) {
        return;
    }

    WaitForGpu();

    for (UINT i = 0; i < kFrameCount; ++i) {
        renderTargets_[i].Reset();
    }

    FAIL(swapChain_->ResizeBuffers(kFrameCount, static_cast<UINT>(width),
                                   static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0));

    frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
    CreateRenderTargets();

    width_ = width;
    height_ = height;
    ChalkLogger::Get().InfoF("Swap chain resized to %dx%d.", width, height);
}

void ChalkGraphics::Finalize() {
    if (initialized_) {
        WaitForGpu();
    }
    ReleaseResources();
    initialized_ = false;
}

void ChalkGraphics::ReleaseResources() {
    if (fenceEvent_ != nullptr) {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }

    vertexBuffer_.Reset();
    pipelineState_.Reset();
    rootSignature_.Reset();
    commandList_.Reset();
    for (UINT i = 0; i < kFrameCount; ++i) {
        commandAllocators_[i].Reset();
        renderTargets_[i].Reset();
    }
    rtvHeap_.Reset();
    swapChain_.Reset();
    fence_.Reset();
    commandQueue_.Reset();
    device_.Reset();
    factory_.Reset();

    rtvDescriptorSize_ = 0;
    frameIndex_ = 0;
    width_ = 0;
    height_ = 0;
    vertexBufferView_ = { };
    for (UINT i = 0; i < kFrameCount; ++i) {
        frameFenceValues_[i] = 0;
    }
}
