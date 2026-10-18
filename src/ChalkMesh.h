#pragma once

#include "ChalkMath.h"

#include <windows.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <vector>

struct ChalkVertex {
    ChalkMath::XMFLOAT3 position;
    ChalkMath::XMFLOAT3 normal;
    ChalkMath::XMFLOAT2 texcoord;
    ChalkMath::XMFLOAT3 tangent;
};

struct ChalkSubmesh {
    UINT indexOffset = 0;
    UINT indexCount = 0;
    UINT materialIndex = 0;
};

class ChalkMesh {
public:
    ChalkMesh();
    ~ChalkMesh();

    ChalkMesh(const ChalkMesh&) = delete;
    ChalkMesh& operator=(const ChalkMesh&) = delete;

    bool Create(ID3D12Device* device,
                const ChalkVertex* vertices,
                UINT vertexCount,
                const std::uint32_t* indices,
                UINT indexCount,
                const ChalkSubmesh* submeshes,
                UINT submeshCount,
                const wchar_t* name = nullptr);

    void Reset();

    bool IsValid() const noexcept;

    UINT VertexCount() const noexcept { return vertexCount_; }
    UINT IndexCount() const noexcept { return indexCount_; }
    UINT SubmeshCount() const noexcept { return static_cast<UINT>(submeshes_.size()); }

    const ChalkSubmesh& GetSubmesh(UINT index) const noexcept { return submeshes_[index]; }

    const D3D12_VERTEX_BUFFER_VIEW& VertexBufferView() const noexcept { return vertexBufferView_; }
    const D3D12_INDEX_BUFFER_VIEW& IndexBufferView() const noexcept { return indexBufferView_; }

    void Draw(ID3D12GraphicsCommandList* commandList) const;
    void DrawSubmesh(ID3D12GraphicsCommandList* commandList, UINT submeshIndex) const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = { };
    D3D12_INDEX_BUFFER_VIEW indexBufferView_ = { };
    UINT vertexCount_ = 0;
    UINT indexCount_ = 0;
    std::vector<ChalkSubmesh> submeshes_;
};
