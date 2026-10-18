#include "ChalkGraphics.h"

#include "ChalkBarrier.h"
#include "ChalkError.h"
#include "ChalkLogger.h"
#include "ChalkResource.h"

#include <d3dcompiler.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>
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

constexpr DXGI_FORMAT kDepthStencilFormat = DXGI_FORMAT_D32_FLOAT;

constexpr UINT kRootParameterTransform = 0;
constexpr UINT kRootParameterMaterial = 1;
constexpr UINT kRootParameterLight = 2;
constexpr UINT kTransformStride = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

struct TransformConstants {
    ChalkMath::XMFLOAT4X4 world;
    ChalkMath::XMFLOAT4X4 worldViewProj;
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
        if (!CreateDepthStencilBuffer(width, height)) {
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
        if (!CreateSceneResources()) {
            return false;
        }

        width_ = width;
        height_ = height;
        camera_.SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
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

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = { };
    dsvHeapDesc.NumDescriptors = kFrameCount;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    FAIL(device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap_)));

    rtvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
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

bool ChalkGraphics::CreateDepthStencilBuffer(int width, int height) {
    wchar_t name[64] = { };
    for (UINT i = 0; i < kFrameCount; ++i) {
        swprintf_s(name, L"DepthStencil[%u]", i);
        if (!depthStencils_[i].CreateDepthBuffer(device_.Get(), dsvHeap_.Get(), i,
                                                 static_cast<UINT>(width),
                                                 static_cast<UINT>(height),
                                                 kDepthStencilFormat, name)) {
            return false;
        }
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
    return fence_.Initialize(device_.Get(), kFrameCount);
}

bool ChalkGraphics::CreatePipeline() {
    D3D12_ROOT_PARAMETER rootParameters[3] = { };
    for (UINT i = 0; i < 3; ++i) {
        rootParameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[i].Descriptor.ShaderRegister = i;
        rootParameters[i].Descriptor.RegisterSpace = 0;
        rootParameters[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = { };
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ChalkVertex, position),
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ChalkVertex, normal),
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ChalkVertex, texcoord),
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ChalkVertex, tangent),
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

    D3D12_DEPTH_STENCIL_DESC depthStencilState = { };
    depthStencilState.DepthEnable = TRUE;
    depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencilState.StencilEnable = FALSE;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = { };
    pipelineDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineDesc.pRootSignature = rootSignature_.Get();
    pipelineDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    pipelineDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    pipelineDesc.RasterizerState = rasterizerState;
    pipelineDesc.BlendState = blendState;
    pipelineDesc.DepthStencilState = depthStencilState;
    pipelineDesc.SampleMask = UINT_MAX;
    pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.NumRenderTargets = 1;
    pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineDesc.DSVFormat = kDepthStencilFormat;
    pipelineDesc.SampleDesc.Count = 1;
    FAIL(device_->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState_)));

    return true;
}

bool ChalkGraphics::CreateUploadBuffer(UINT64 size, const wchar_t* name,
                                       Microsoft::WRL::ComPtr<ID3D12Resource>& outResource) {
    D3D12_HEAP_PROPERTIES heapProps = { };
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC bufferDesc = { };
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = size;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    FAIL(device_->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                          IID_PPV_ARGS(&outResource)));
    if (name != nullptr) {
        outResource->SetName(name);
    }
    return true;
}

bool ChalkGraphics::CreateSceneResources() {
    const float h = 1.0f;

    std::vector<ChalkVertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve(24);
    indices.reserve(36);

    auto pushFace = [&vertices, &indices](const ChalkMath::XMFLOAT3& a,
                                          const ChalkMath::XMFLOAT3& b,
                                          const ChalkMath::XMFLOAT3& c,
                                          const ChalkMath::XMFLOAT3& d,
                                          const ChalkMath::XMFLOAT3& normal,
                                          const ChalkMath::XMFLOAT3& tangent) {
        const std::uint32_t base = static_cast<std::uint32_t>(vertices.size());
        vertices.push_back({ a, normal, { 0.0f, 1.0f }, tangent });
        vertices.push_back({ b, normal, { 0.0f, 0.0f }, tangent });
        vertices.push_back({ c, normal, { 1.0f, 0.0f }, tangent });
        vertices.push_back({ d, normal, { 1.0f, 1.0f }, tangent });
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    };

    pushFace({ h, -h, -h }, { h, h, -h }, { h, h, h }, { h, -h, h },
             { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    pushFace({ -h, -h, h }, { -h, h, h }, { -h, h, -h }, { -h, -h, -h },
             { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
    pushFace({ -h, h, -h }, { -h, h, h }, { h, h, h }, { h, h, -h },
             { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });
    pushFace({ -h, -h, h }, { -h, -h, -h }, { h, -h, -h }, { h, -h, h },
             { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });
    pushFace({ -h, -h, h }, { h, -h, h }, { h, h, h }, { -h, h, h },
             { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f });
    pushFace({ h, -h, -h }, { -h, -h, -h }, { -h, h, -h }, { h, h, -h },
             { 0.0f, 0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f });

    const ChalkSubmesh submesh = { 0, 36, 0 };
    if (!cubeMesh_.Create(device_.Get(), vertices.data(),
                          static_cast<UINT>(vertices.size()), indices.data(),
                          static_cast<UINT>(indices.size()), &submesh, 1, L"CubeMesh")) {
        return false;
    }

    for (UINT i = 0; i < kMaterialCount; ++i) {
        wchar_t materialName[32] = { };
        swprintf_s(materialName, L"SceneMaterial[%u]", i);
        if (!materials_[i].Create(device_.Get(), materialName)) {
            return false;
        }
    }
    materials_[0].SetBaseColor({ 0.90f, 0.90f, 0.92f, 1.0f });
    materials_[1].SetBaseColor({ 0.90f, 0.35f, 0.30f, 1.0f });
    for (UINT i = 0; i < kMaterialCount; ++i) {
        if (!materials_[i].Upload()) {
            return false;
        }
    }

    const UINT64 objectBufferSize = static_cast<UINT64>(kFrameCount) *
                                    static_cast<UINT64>(kMaxSceneObjects) * kTransformStride;
    if (!CreateUploadBuffer(objectBufferSize, L"ObjectConstants", objectConstantBuffer_)) {
        return false;
    }

    const UINT64 lightBufferSize = static_cast<UINT64>(kFrameCount) * kTransformStride;
    if (!CreateUploadBuffer(lightBufferSize, L"LightConstants", lightConstantBuffer_)) {
        return false;
    }

    scene_.Clear();
    scene_.Add(L"CenterCube", &cubeMesh_, &materials_[0]);
    ChalkSceneObject& offset = scene_.Add(L"OffsetCube", &cubeMesh_, &materials_[1]);
    DirectX::XMStoreFloat4x4(&offset.world, ChalkMath::Translation(2.2f, 0.0f, 0.0f));

    scene_.Light().direction = { -0.4f, -1.0f, -0.6f, 0.0f };

    camera_.SetPosition({ 3.0f, 2.5f, 5.5f });
    camera_.SetTarget({ 0.5f, 0.0f, 0.0f });
    camera_.SetUp({ 0.0f, 1.0f, 0.0f });

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
    ChalkBarrier::Transition(
        commandList_.Get(),
        renderTargets_[frameIndex_].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(frameIndex_) * rtvDescriptorSize_;

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
    dsvHandle.ptr += static_cast<SIZE_T>(frameIndex_) * dsvDescriptorSize_;

    commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    commandList_->ClearRenderTargetView(rtvHandle, kClearColor, 0, nullptr);
    commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

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

    const ChalkDirectionalLightConstants lightConstants = scene_.BuildLightConstants();
    const UINT64 lightOffset = static_cast<UINT64>(frameIndex_) * kTransformStride;
    void* lightMapped = nullptr;
    const D3D12_RANGE lightReadRange = { 0, 0 };
    FAIL(lightConstantBuffer_->Map(0, &lightReadRange, &lightMapped));
    memcpy(static_cast<std::uint8_t*>(lightMapped) + lightOffset, &lightConstants,
           sizeof(lightConstants));
    lightConstantBuffer_->Unmap(0, nullptr);
    commandList_->SetGraphicsRootConstantBufferView(
        kRootParameterLight, lightConstantBuffer_->GetGPUVirtualAddress() + lightOffset);

    const ChalkMath::XMMATRIX viewProjection = camera_.GetViewProjectionMatrix();
    const std::vector<ChalkSceneObject>& objects = scene_.Objects();
    const UINT objectCount = objects.size() < kMaxSceneObjects
                                 ? static_cast<UINT>(objects.size())
                                 : kMaxSceneObjects;

    for (UINT i = 0; i < objectCount; ++i) {
        const ChalkSceneObject& object = objects[i];
        if (object.mesh == nullptr || object.material == nullptr) {
            continue;
        }

        const ChalkMath::XMMATRIX world = DirectX::XMLoadFloat4x4(&object.world);
        const ChalkMath::XMMATRIX worldViewProj = ChalkMath::Multiply(world, viewProjection);

        TransformConstants transform = { };
        transform.world = ChalkMath::ToUploadMatrix(world);
        transform.worldViewProj = ChalkMath::ToUploadMatrix(worldViewProj);

        const UINT64 transformOffset =
            (static_cast<UINT64>(frameIndex_) * kMaxSceneObjects + i) * kTransformStride;
        void* transformMapped = nullptr;
        const D3D12_RANGE transformReadRange = { 0, 0 };
        FAIL(objectConstantBuffer_->Map(0, &transformReadRange, &transformMapped));
        memcpy(static_cast<std::uint8_t*>(transformMapped) + transformOffset, &transform,
               sizeof(transform));
        objectConstantBuffer_->Unmap(0, nullptr);

        commandList_->SetGraphicsRootConstantBufferView(
            kRootParameterTransform,
            objectConstantBuffer_->GetGPUVirtualAddress() + transformOffset);
        object.material->Bind(commandList_.Get(), kRootParameterMaterial);
        object.mesh->Draw(commandList_.Get());
    }

    ChalkBarrier::Transition(
        commandList_.Get(),
        renderTargets_[frameIndex_].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    FAIL(commandList_->Close());
}

void ChalkGraphics::MoveToNextFrame() {
    const uint64_t signalledValue = fence_.Signal(commandQueue_.Get(), frameIndex_);

    frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
    fence_.WaitForFrame(frameIndex_);
    fence_.SetFrameValue(frameIndex_, signalledValue + 1);
}

void ChalkGraphics::WaitForGpu() {
    if (!commandQueue_ || !fence_.IsInitialized()) {
        return;
    }

    fence_.WaitForGpu(commandQueue_.Get(), frameIndex_);
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
        depthStencils_[i].Reset();
    }

    FAIL(swapChain_->ResizeBuffers(kFrameCount, static_cast<UINT>(width),
                                   static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0));

    frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
    if (!CreateRenderTargets() || !CreateDepthStencilBuffer(width, height)) {
        ChalkLogger::Get().Error("Failed to recreate render targets after resize.");
        return;
    }

    width_ = width;
    height_ = height;
    camera_.SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
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
    objectConstantBuffer_.Reset();
    lightConstantBuffer_.Reset();
    cubeMesh_.Reset();
    for (UINT i = 0; i < kMaterialCount; ++i) {
        materials_[i].Reset();
    }
    scene_.Clear();
    pipelineState_.Reset();
    rootSignature_.Reset();
    commandList_.Reset();
    for (UINT i = 0; i < kFrameCount; ++i) {
        commandAllocators_[i].Reset();
        renderTargets_[i].Reset();
        depthStencils_[i].Reset();
    }
    rtvHeap_.Reset();
    dsvHeap_.Reset();
    swapChain_.Reset();
    fence_.Shutdown();
    commandQueue_.Reset();
    device_.Reset();
    factory_.Reset();

    rtvDescriptorSize_ = 0;
    dsvDescriptorSize_ = 0;
    frameIndex_ = 0;
    width_ = 0;
    height_ = 0;
}
