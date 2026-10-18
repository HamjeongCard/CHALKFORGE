#include "ChalkBarrier.h"

#include "ChalkLogger.h"

void ChalkBarrier::Transition(
    ID3D12GraphicsCommandList* commandList,
    ID3D12Resource* resource,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after) {
    if (commandList == nullptr || resource == nullptr) {
        ChalkLogger::Get().Error("ChalkBarrier::Transition received a null argument.");
        return;
    }

    if (before == after) {
        return;
    }

    D3D12_RESOURCE_BARRIER barrier = { };
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;

    commandList->ResourceBarrier(1, &barrier);
}
