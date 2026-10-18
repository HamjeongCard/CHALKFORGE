PHASE 03 — Direct3D 12 초기화 및 기본 렌더링

**목표**: Direct3D 12 디바이스/스왑 체인을 초기화하고, 파이프라인을 구성하여 단일 삼각형을 백 버퍼에 렌더링한다. GPU 동기화와 창 리사이즈 처리를 포함한다.

**구현 완료 상태**: 구현 및 내부 검증 완료 (Implemented, Build/Runtime Verified)
- 단, 삼각형 화면 출력 및 리사이즈 동작의 육안 확인은 미검증 (사용자 확인 항목)

**확인된 상태 (AI 직접 실행 검증)**:
- 빌드: Debug/Release 구성 모두 성공 (경고 없음, C4244 해결 후)
- 실행: `build\Debug\CHALKFORGE.exe` 무크래시 실행, 메시지 루프 지속 동작 확인
- 로그(`chalkforge.log`): `CHALKFORGE initializing` → `D3D12 debug layer enabled.` → `D3D12 graphics initialized (1280x720).` (오류 없음)
- 셰이더 복사: `build\Debug\shaders\TriangleVS.hlsl`, `TrianglePS.hlsl` 배치 확인

**미검증 상태 (사용자 확인 필요)**:
- 삼각형이 실제로 화면에 표시되는지 (육안)
- 창 크기 조절 시 백 버퍼/뷰포트가 정상 갱신되는지

**작업 단계 및 변경 파일**:

1. **Step A — 문서 정합화**
   - 수정: `ROADMAP.md`, `CODE_GUIDE.md`, `TASKS.md`, `DECISIONS.md`, `PROJECT_PLAN.md`
   - PHASE 03 범위/설계 결정을 문서에 반영

2. **Step B — `build/` Git 추적 해제**
   - 수정: `.gitignore` (`build/` 추가), `git rm -r --cached build` (26개 인덱스 제거, 디스크 보존)

3. **Step C — PHASE 01 기반 파일 구현**
   - 신규: `src/ChalkConfig.h`, `src/ChalkLogger.h`/`.cpp`, `src/ChalkError.h`/`.cpp`, `src/ChalkMath.h`, `src/ChalkApp.h`
   - 수정: `src/ChalkApp.cpp`, `CMakeLists.txt`

4. **Step D — CMake 확장**
   - 수정: `CMakeLists.txt`
   - `d3d12`/`dxgi`/`d3dcompiler` 링크, `/utf-8`, Debug 시 `CHALK_DEBUG_LAYER` 정의, `NOMINMAX`

5. **Step E — Win32 리팩터링 + UNICODE/DPI**
   - 신규: `src/ChalkWindow.h`/`.cpp`
   - 수정: `src/ChalkApp.h`/`.cpp` (ChalkApplication + `wWinMain`, Per-Monitor V2 DPI), `CMakeLists.txt`

6. **Step F — D3D12 초기화 및 삼각형 렌더링 (핵심)**
   - 신규: `src/ChalkGraphics.h`/`.cpp`, `shaders/TriangleVS.hlsl`, `shaders/TrianglePS.hlsl`
   - 수정: `src/ChalkWindow.h`/`.cpp` (PeekMessage 렌더 루프), `src/ChalkApp.h`/`.cpp` (그래픽스 연결/해제/리사이즈), `CMakeLists.txt` (셰이더 복사)
   - 수정: `CODE_GUIDE.md` (ChalkGraphics/초기화·렌더링 흐름), `TASKS.md`

7. **Step G — 단계 문서화**
   - 신규: `docs/phases/PHASE_03.md` (본 문서)
   - 수정: `CHANGELOG.md`, `TEST_LOG.md`, `PROMPT_LOG.md`, `TASKS.md`

**구현 내용 (Step F 핵심)**:

1. **`ChalkGraphics` 클래스** (`src/ChalkGraphics.h`/`.cpp`):
   - 단일 클래스 + 내부 캡슐화. 복사 금지. 세부 자원 분리는 PHASE 04.
   - public: `Initialize(HWND,int,int)`, `Finalize()`, `RenderFrame()`, `OnResize(int,int)`, `WaitForGpu()`, `IsInitialized()`
   - private 생성 단계: `CreateDeviceAndQueue` / `CreateSwapChain` / `CreateDescriptorHeaps` / `CreateRenderTargets` / `CreateCommandObjects` / `CreateFence` / `CreatePipeline` / `CreateTriangleGeometry`

2. **디바이스 및 명령 큐** (`CreateDeviceAndQueue`):
   - `CreateDXGIFactory2` → 어댑터 열거(소프트웨어 스킵) → `D3D12CreateDevice`
   - 피처 레벨 폴백 `{12_0, 11_1, 11_0}`
   - Debug 시 `D3D12GetDebugInterface` + `EnableDebugLayer()`
   - `D3D12_COMMAND_LIST_TYPE_DIRECT` 큐 생성

3. **스왑 체인** (`CreateSwapChain`):
   - `DXGI_SWAP_CHAIN_DESC1`: `BufferCount=kFrameCount(2)`, `R8G8B8A8_UNORM`, `FLIP_DISCARD`, `SCALING_STRETCH`
   - `CreateSwapChainForHwnd` + `MakeWindowAssociation(DXGI_MWA_NO_ALT_ENTER)` → `IDXGISwapChain3`
   - `frameIndex_ = GetCurrentBackBufferIndex()`

4. **RTV 힙 및 렌더 타깃** (`CreateDescriptorHeaps`/`CreateRenderTargets`):
   - `D3D12_DESCRIPTOR_HEAP_TYPE_RTV`, `NumDescriptors=kFrameCount`
   - 각 백 버퍼에 대해 `CreateRenderTargetView`, `rtvDescriptorSize_` 계산

5. **루트 서명 및 파이프라인** (`CreatePipeline`):
   - 빈 파라미터 루트 서명 + `ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT`
   - `D3DCompile`로 `TriangleVS.hlsl`/`TrianglePS.hlsl` 런타임 컴파일 (`vs_5_0`/`ps_5_0`)
   - 익명 네임스페이스 `struct Vertex { float position[3]; float color[4]; }` + `offsetof` 기반 INPUT_ELEMENT_DESC
   - `D3D12_RASTERIZER_DESC`/`D3D12_BLEND_DESC` 기본값 직접 작성 (CD3DX12 헬퍼 미사용), `CullMode=NONE`, `DepthEnable=FALSE`

6. **정점 버퍼** (`CreateTriangleGeometry`):
   - 삼각형 3개 NDC 좌표 (빨강/초록/파랑), 업로드 힙(`GENERIC_READ`) + `Map/memcpy/Unmap`

7. **프레임 동기화** (`RenderFrame`/`PopulateCommandList`/`MoveToNextFrame`/`WaitForGpu`):
   - switch: allocator Reset → list Reset → RTV barrier(PRESENT→RT) → OMSetRenderTargets → ClearRTV → SetGraphicsRootSignature/SetPipelineState/IASetPrimitiveTopology/RSSetViewports/RSSetScissorRects/IASetVertexBuffers/DrawInstanced(3,1,0,0) → barrier(RT→PRESENT) → Close
   - ExecuteCommandLists → Present(1,0) → MoveToNextFrame(Fence Signal/SetEventOnCompletion/Wait)

8. **리사이즈** (`OnResize`):
   - `WaitForGpu()` → renderTargets Reset → `swapChain_->ResizeBuffers(...)` → CreateRenderTargets → 상태 갱신 + 로그

9. **ChalkWindow/ChalkApplication 연결**:
   - `ChalkWindow::ProcessMessages(const std::function<void()>& onIdle)`: `PeekMessageW(PM_REMOVE)` 루프 + `WM_QUIT` 조기 반환 + `onIdle()` 호출 (블로킹 `GetMessageW` 제거)
   - `ChalkApplication::Run`: `window_->ProcessMessages([this]{ graphics_->RenderFrame(); })` + 전체 try/catch
   - `ChalkApplication::OnResize`: `graphics_ && IsInitialized() && w/h>0`일 때만 전달

**CMakeLists.txt 변경 사항 (주요)**:
```cmake
add_executable(CHALKFORGE
    src/ChalkApp.cpp
    src/ChalkLogger.cpp
    src/ChalkError.cpp
    src/ChalkGraphics.cpp
    src/ChalkWindow.cpp
)

if(MSVC)
    target_compile_options(CHALKFORGE PRIVATE /utf-8)
endif()

target_compile_definitions(CHALKFORGE PRIVATE
    UNICODE _UNICODE NOMINMAX
    WINVER=0x0A00 _WIN32_WINNT=0x0A00
)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(CHALKFORGE PRIVATE CHALK_DEBUG_LAYER)
endif()

target_link_libraries(CHALKFORGE PRIVATE d3d12 dxgi d3dcompiler)

add_custom_command(TARGET CHALKFORGE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:CHALKFORGE>/shaders
)
```

**빌드 및 실행 명령**:
- 빌드: `cmake --build build --config Debug` / `--config Release`
- 실행: `build\Debug\CHALKFORGE.exe`

**오류 해결 기록**:
- C4244(wide→narrow 변환 경고): `src/ChalkGraphics.cpp` 익명 네임스페이스에 로컬 `WideToUtf8(const std::wstring&)`(`WideCharToMultiByte(CP_UTF8, ...)` 2회 호출) 추가 후 두 호출부 교체 → 경고 없이 빌드 성공

**기술적 위험**:
- 스왑 체인 리사이즈와 GPU 동기화(WaitForGpu) 순서 오류 시 크래시 가능 → 리사이즈 경로 사용자 육안 검증 필요
- FLIP_DISCARD 모델에서 백 버퍼 접근 전 PRESENT→RT barrier 누락 시 디버그 레이어 경고

**학습 목표**: D3D12 디바이스/큐/스왑 체인, 디스크립터 힙, 루트 서명, PSO, 리소스 배리어, Fence 기반 GPU 동기화, HLSL 런타임 컴파일, PeekMessage 기반 렌더 루프

**검증 절차**:
1. 프로젝트 성공적 빌드 (Debug/Release) ✅
2. 실행 파일 무크래시 실행 및 초기화 로그 확인 ✅
3. 삼각형 화면 표시 확인 ⬜ (미검증 — 사용자 확인 필요)
4. 창 리사이즈 시 정상 갱신 확인 ⬜ (미검증 — 사용자 확인 필요)

**수락 기준**:
1. Debug/Release 빌드 성공 ✅
2. D3D12 초기화 성공 로그 (1280x720) ✅
3. 삼각형 렌더링 (육안) ⬜
4. 리사이즈 정상 동작 ⬜

**미해결 이슈**:
- 삼각형 화면 출력 및 리사이즈 동작의 육안 검증 미완료
- 깊이 버퍼 미구현 (PHASE 04/05)
- GPU 자원 세부 분리 및 프레임 동기화 고도화 (PHASE 04)
