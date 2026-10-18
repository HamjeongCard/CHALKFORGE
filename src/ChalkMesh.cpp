#include "ChalkMesh.h"

#include "ChalkError.h"
#include "ChalkLogger.h"

#include <cstring>
#include <string>

ChalkMesh::ChalkMesh() = default;

ChalkMesh::~ChalkMesh() {
    Reset();
}

bool ChalkMesh::Create(ID3D12Device* device,
                       const ChalkVertex* vertices,
                       UINT vertexCount,
                       const std::uint32_t* indices,
                       UINT indexCount,
                       const ChalkSubmesh* submeshes,
                       UINT submeshCount,
                       const wchar_t* name) {
    if (device == nullptr || vertices == nullptr || vertexCount == 0
        || indices == nullptr || indexCount == 0) {
        ChalkLogger::Get().Error("ChalkMesh::Create received invalid arguments.");
        return false;
    }

    Reset();

    const UINT64 vertexBufferSize = static_cast<UINT64>(vertexCount) * sizeof(ChalkVertex);
    const UINT64 indexBufferSize = static_cast<UINT64>(indexCount) * sizeof(std::uint32_t);

    D3D12_HEAP_PROPERTIES heapProps = { };
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC bufferDesc = { };
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = vertexBufferSize;
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
                                         IID_PPV_ARGS(&vertexBuffer_)));

    bufferDesc.Width = indexBufferSize;
    FAIL(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                         D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                         IID_PPV_ARGS(&indexBuffer_)));

    if (name != nullptr) {
        vertexBuffer_->SetName(name);
        indexBuffer_->SetName((std::wstring(name) + L" Indices").c_str());
    }

    void* mapped = nullptr;
    const D3D12_RANGE readRange = { 0, 0 };

    FAIL(vertexBuffer_->Map(0, &readRange, &mapped));
    std::memcpy(mapped, vertices, static_cast<std::size_t>(vertexBufferSize));
    vertexBuffer_->Unmap(0, nullptr);

    mapped = nullptr;
    FAIL(indexBuffer_->Map(0, &readRange, &mapped));
    std::memcpy(mapped, indices, static_cast<std::size_t>(indexBufferSize));
    indexBuffer_->Unmap(0, nullptr);

    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.StrideInBytes = sizeof(ChalkVertex);
    vertexBufferView_.SizeInBytes = static_cast<UINT>(vertexBufferSize);

    indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
    indexBufferView_.SizeInBytes = static_cast<UINT>(indexBufferSize);

    vertexCount_ = vertexCount;
    indexCount_ = indexCount;

    submeshes_.clear();
    if (submeshes != nullptr && submeshCount > 0) {
        submeshes_.assign(submeshes, submeshes + submeshCount);
    }

    ChalkLogger::Get().InfoF("Mesh created (%u vertices, %u indices, %llu submeshes).",
                             vertexCount_, indexCount_,
                             static_cast<unsigned long long>(submeshes_.size()));
    return true;
}

void ChalkMesh::Reset() {
    if (vertexBuffer_ || indexBuffer_) {
        ChalkLogger::Get().Info("ChalkMesh released.");
    }

    vertexBuffer_.Reset();
    indexBuffer_.Reset();
    vertexBufferView_ = { };
    indexBufferView_ = { };
    vertexCount_ = 0;
    indexCount_ = 0;
    submeshes_.clear();
}

bool ChalkMesh::IsValid() const noexcept {
    return vertexBuffer_ != nullptr && indexBuffer_ != nullptr;
}

void ChalkMesh::Draw(ID3D12GraphicsCommandList* commandList) const {
    if (commandList == nullptr || !IsValid()) {
        return;
    }

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetIndexBuffer(&indexBufferView_);
    commandList->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);
}

void ChalkMesh::DrawSubmesh(ID3D12GraphicsCommandList* commandList, UINT submeshIndex) const {
    if (commandList == nullptr || !IsValid() || submeshIndex >= submeshes_.size()) {
        return;
    }

    const ChalkSubmesh& submesh = submeshes_[submeshIndex];

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetIndexBuffer(&indexBufferView_);
    commandList->DrawIndexedInstanced(submesh.indexCount, 1, submesh.indexOffset, 0, 0);
}
