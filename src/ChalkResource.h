#pragma once

#include <windows.h>
#include <d3d12.h>
#include <wrl/client.h>

class ChalkResource {
public:
    ChalkResource();
    ~ChalkResource();

    ChalkResource(const ChalkResource&) = delete;
    ChalkResource& operator=(const ChalkResource&) = delete;

    bool CreateDepthBuffer(ID3D12Device* device,
                           ID3D12DescriptorHeap* dsvHeap,
                           UINT dsvIndex,
                           UINT width,
                           UINT height,
                           DXGI_FORMAT format,
                           const wchar_t* name = nullptr);

    void Reset();

    ID3D12Resource* Get() const noexcept { return resource_.Get(); }
    bool IsValid() const noexcept { return resource_ != nullptr; }
    D3D12_RESOURCE_STATES State() const noexcept { return state_; }
    void SetState(D3D12_RESOURCE_STATES state) noexcept { state_ = state; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    D3D12_RESOURCE_STATES state_ = D3D12_RESOURCE_STATE_COMMON;
};
