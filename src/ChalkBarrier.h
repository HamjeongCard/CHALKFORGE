#pragma once

#include <d3d12.h>

class ChalkBarrier {
public:
    ChalkBarrier() = delete;

    static void Transition(
        ID3D12GraphicsCommandList* commandList,
        ID3D12Resource* resource,
        D3D12_RESOURCE_STATES before,
        D3D12_RESOURCE_STATES after);
};
