#include "ChalkMaterial.h"

#include "ChalkError.h"
#include "ChalkLogger.h"

#include <cstdint>
#include <cstring>

namespace {
constexpr UINT64 kConstantBufferSize = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
}

ChalkMaterial::ChalkMaterial() = default;

ChalkMaterial::~ChalkMaterial() {
    Reset();
}

bool ChalkMaterial::Create(ID3D12Device* device, const wchar_t* name) {
    if (device == nullptr) {
        ChalkLogger::Get().Error("ChalkMaterial::Create received an invalid device.");
        return false;
    }

    Reset();

    D3D12_HEAP_PROPERTIES heapProps = { };
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC bufferDesc = { };
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = kConstantBufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    FAIL(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                         D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                         IID_PPV_ARGS(&constantBuffer_)));

    if (name != nullptr) {
        constantBuffer_->SetName(name);
    }

    if (!Upload()) {
        Reset();
        return false;
    }

    ChalkLogger::Get().InfoF("Material created (%llu byte constant buffer, %llu byte payload).",
                             static_cast<unsigned long long>(kConstantBufferSize),
                             static_cast<unsigned long long>(sizeof(ChalkMaterialConstants)));
    return true;
}

void ChalkMaterial::Reset() {
    if (constantBuffer_) {
        ChalkLogger::Get().Info("ChalkMaterial released.");
    }

    constantBuffer_.Reset();
}

bool ChalkMaterial::IsValid() const noexcept {
    return constantBuffer_ != nullptr;
}

bool ChalkMaterial::Upload() {
    if (constantBuffer_ == nullptr) {
        return false;
    }

    void* mapped = nullptr;
    const D3D12_RANGE readRange = { 0, 0 };

    FAIL(constantBuffer_->Map(0, &readRange, &mapped));
    std::memcpy(mapped, &constants_, sizeof(ChalkMaterialConstants));
    constantBuffer_->Unmap(0, nullptr);

    return true;
}

void ChalkMaterial::Bind(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex) const {
    if (commandList == nullptr || !IsValid()) {
        return;
    }

    commandList->SetGraphicsRootConstantBufferView(rootParameterIndex,
                                                   constantBuffer_->GetGPUVirtualAddress());
}
