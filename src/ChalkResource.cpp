#include "ChalkResource.h"

#include "ChalkError.h"
#include "ChalkLogger.h"

ChalkResource::ChalkResource() = default;

ChalkResource::~ChalkResource() {
    Reset();
}

bool ChalkResource::CreateDepthBuffer(ID3D12Device* device,
                                      ID3D12DescriptorHeap* dsvHeap,
                                      UINT dsvIndex,
                                      UINT width,
                                      UINT height,
                                      DXGI_FORMAT format,
                                      const wchar_t* name) {
    if (device == nullptr || dsvHeap == nullptr || width == 0 || height == 0) {
        ChalkLogger::Get().Error("ChalkResource::CreateDepthBuffer received invalid arguments.");
        return false;
    }

    Reset();

    const UINT dsvDescriptorSize =
        device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    dsvHandle.ptr += static_cast<SIZE_T>(dsvIndex) * dsvDescriptorSize;

    D3D12_HEAP_PROPERTIES heapProps = { };
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC depthDesc = { };
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = static_cast<UINT64>(width);
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = format;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = { };
    clearValue.Format = format;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    FAIL(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc,
                                         D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
                                         IID_PPV_ARGS(&resource_)));

    if (name != nullptr) {
        resource_->SetName(name);
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
    dsvDesc.Format = format;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    device->CreateDepthStencilView(resource_.Get(), &dsvDesc, dsvHandle);

    state_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    ChalkLogger::Get().InfoF("Depth buffer created (%ux%u).", width, height);
    return true;
}

void ChalkResource::Reset() {
    if (resource_) {
        ChalkLogger::Get().Info("ChalkResource released.");
    }

    resource_.Reset();
    state_ = D3D12_RESOURCE_STATE_COMMON;
}
