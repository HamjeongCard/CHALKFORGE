CODE_GUIDE.md - CHALKFORGE 코드 가이드

## 엔트리 포인트
### wWinMain (Win32 애플리케이션 시작점)
- **위치**: PHASE 03에서 UNICODE 엔트리 포인트로 전환 (src/ChalkApp.cpp)
- **역할**: 애플리케이션 시작점; ChalkApplication 수명 주기 호출만 담당 (창 생성·메시 펌프는 ChalkApplication/ChalkWindow로 위임)
- **호출 순서**:
  1. `ChalkApplication` 인스턴스 생성
  2. `Initialize(hInstance, nCmdShow)`: DPI 인식 설정, 창 생성, (PHASE 03 Step F부터) D3D12 초기화
  3. `Run()`: 메시 펌프 진입
  4. `Finalize()`: 자원 정리, 로거 종료
  5. 프로세스 종료 코드 반환

## planned 아키텍처의 핵심 클래스
### ChalkApplication
- **파일**: PHASE 03에서 구현 (src/ChalkApp.h/.cpp)
- **담당**: 메인 애플리케이션 오케스트레이션, 수명 주기 관리, 창/그래픽스 연결
- **주요 메서드**:
  - `Initialize(HINSTANCE, int)`: 로깅 초기화, DPI 인식 설정, 창 생성, (PHASE 03 Step F부터) D3D12 초기화
  - `Run()`: 메시 펌프 실행
  - `Finalize()`: 창 정리, 로거 종료
  - `OnResize(int, int)`: 창 크기 변경 시 크기 저장 및 (PHASE 03 Step F부터) 스왑 체인/리소스 재조정

### ChalkGraphics
- **파일**: PHASE 03에 구현 (src/ChalkGraphics.h/.cpp)
- **담당**: D3D12 자원 통합 관리 — 디바이스, 명령 큐, 명령 할당자/목록, 스왑 체인, RTV 힙, 루트 서명, PSO, 정점 버퍼, 펜스를 단일 클래스 내부에 캡슐화 (DECISIONS.md 참조)
- **주요 메서드**:
  - `Initialize(HWND, int, int)`: 디바이스/큐/스왑 체인/파이프라인/정점 버퍼 초기화. 실패 시 try/catch로 로그 후 자원 해제하고 false 반환
  - `RenderFrame()`: 할당자/커맨드 리스트 리셋 → 백 버퍼 기록 → `ExecuteCommandLists()` → `Present(1,0)` → `MoveToNextFrame()`
  - `OnResize(int, int)`: `WaitForGpu()` 후 스왑 체인 버퍼 재생성 및 RTV 재생성
  - `WaitForGpu()`: 펜스 기반 GPU 동기화
  - `Finalize()`: GPU 대기 후 자원 해제 (RTV → 스왑 체인 → 디바이스 순)
- **내부 메서드**: `CreateDeviceAndQueue`/`CreateSwapChain`/`CreateDescriptorHeaps`/`CreateRenderTargets`/`CreateCommandObjects`/`CreateFence`/`CreatePipeline`/`CreateTriangleGeometry`/`PopulateCommandList`/`MoveToNextFrame`/`ReleaseResources`
- **셰이더**: `shaders/TriangleVS.hlsl`·`TrianglePS.hlsl`을 실행 시 `D3DCompile`로 컴파일. CMake POST_BUILD가 exe 옆 `shaders\`로 복사

### ChalkFence (PHASE 04)
- **파일**: PHASE 04에 생성 (src/ChalkFence.h/.cpp)
- **담당**: GPU 동기화 펜스/이벤트/per-frame 값 캡슐화. 기존 `ChalkGraphics`의 fence 로직을 이 클래스로 이관
- **주요 메서드**:
  - `Signal(ID3D12CommandQueue*, UINT64)`: 펜스 값으로 큐 신호, `CompletedValue` 반환
  - `WaitForValue(UINT64)`: `SetEventOnCompletion` + `WaitForSingleObject`로 특정 값 대기
  - `WaitForGpu(ID3D12CommandQueue*, UINT64)`: 현재 값까지 GPU 대기
  - `CompletedValue()`: 현재 완료된 펜스 값 조회
- **per-frame 값**: `frameFenceValues_[kFrameCount]`를 내부 관리

### ChalkBarrier (PHASE 04)
- **파일**: PHASE 04에 생성 (src/ChalkBarrier.h/.cpp)
- **담당**: 리소스 상태 전이(barrier) 도우미. 인라인 barrier 구성 코드를 대체
- **주요 메서드**:
  - `Transition(ID3D12GraphicsCommandList*, ID3D12Resource*, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)`: 단일 리소스 상태 전이 배리어 삽입
- **사용 예**: PRESENT→RENDER_TARGET, RENDER_TARGET→PRESENT, DEPTH_WRITE 등

### ChalkResource (PHASE 04)
- **파일**: PHASE 04에 생성 (src/ChalkResource.h/.cpp)
- **담당**: `ID3D12Resource` 래퍼 — 현재 state 추적, 이름/디버그 태그, 수명 등록/해제 로깅(CPU 측 핸들 추적), 깊이 버퍼 생성 도우미
- **주요 필드/메서드**: `Get()`(원시 리소스 반환), `State()`/`SetState()`, `CreateDepthBuffer(ID3D12Device*, DSV 힙, int w, int h, DXGI_FORMAT)`, 이름 설정
- **수명 추적**: 등록/해제 시 `ChalkLogger`로 로깅하여 누수 추적 지원

### ChalkWindow
- **파일**: PHASE 03에서 ChalkApp.cpp 내 WndProc에서 클래스로 분리 (src/ChalkWindow.h/.cpp)
- **담당**: Win32 창 생성 및 메시 처리 (UNICODE, Per-Monitor V2 DPI)
- **주요 메서드**:
  - `Create(HINSTANCE, int, LPCWSTR)`: 창 클래스 등록 및 창 생성 (`WNDCLASSEXW`/`CreateWindowExW`)
  - `ProcessMessages(std::function<void()> onIdle)`: `PeekMessageW`/`DispatchMessageW` 루프. 메시가 없으면 `onIdle()`(렌더 프레임) 호출, `WM_QUIT` 시 종료 코드 반환
  - `OnMessage(UINT, WPARAM, LPARAM)`: 메시 디스패치 (`WM_SIZE` → 크기 콜백, `WM_DESTROY` → 종료, `WM_PAINT` → 배경 채우기)
  - `SetResizeCallback(std::function<void(int,int)>)`: 크기 조정 이벤트 등록
  - `Handle()/Width()/Height()`: 창 핸들·클라이언트 크기 조회

### ChalkMath (PHASE 05 확장)
- **파일**: PHASE 03 최소 헤더(`ChalkMath.h`) → PHASE 05에서 확장 및 `ChalkMath.cpp` 신규 (src/ChalkMath.h/.cpp)
- **담당**: DirectXMath 별칭(`XMFLOAT2/3/4`, `XMFLOAT4X4`, `XMMATRIX`, `XMVECTOR`) 및 수학 유틸리티 — 각도 변환, 항등 행렬, 행렬 곱/전치/역행렬, 투영·뷰 헬퍼
- **행렬 규칙**: CPU는 행-우선(row-major), cbuffer 업로드 직전 `XMMatrixTranspose` (DECISIONS.md 참조)
- **주요 함수**: `DegreesToRadians`/`RadiansToDegrees`, `Identity()`, 전치·역행렬·투영/뷰 변환 헬퍼 (PHASE 05 Step B에서 구현)

### ChalkCamera
- **파일**: PHASE 05에 구현 (src/ChalkCamera.h/.cpp)
- **담당**: 카메라 위치·타겟·업 벡터, 뷰/프로젝션 행렬 생성, 카메라 이동
- **주요 메서드**:
  - `SetPosition`/`SetTarget`/`SetUp`: 카메라 파라미터 설정
  - `GetViewMatrix()`: `LookAtRH` 기반 뷰 행렬 (우측-handed)
  - `GetProjectionMatrix()`: FOV/종횡비/근·원 평면 기반 `PerspectiveFovRH` 투영 행렬 (우측-handed)
  - `GetViewProjectionMatrix()`: 뷰·투영 결합 (셰이더 cbuffer 업로드용)
  - 카메라 이동: WASD 이동 및 마우스 궤도(orbit)

### ChalkMesh
- **파일**: PHASE 05에 구현 (src/ChalkMesh.h/.cpp)
- **담당**: 버텍스/인덱스 버퍼 관리, 서브메쉬(submesh) 구성, 메쉬 로드
- **주요 메서드**: 버텍스/인덱스 데이터 업로드, `DrawIndexed`용 인덱스 버퍼 뷰, 서브메쉬별 렌더 범위 관리, 정점·인덱스 버퍼 생성

### ChalkMaterial
- **파일**: PHASE 05에 구현 (src/ChalkMaterial.h/.cpp)
- **담당**: 머티리얼 Constant Buffer(256-byte 정렬 준수, 색상/방출색/거칠기/금속성 등), 서브셋 ID
- **주요 메서드**: 머티리얼 파라미터 설정(`SetBaseColor`/`SetEmissive`/`SetRoughness`/`SetMetallic`/`SetSpecularPower`/`SetOpacity`), cbuffer 업로드(`Upload`), 루트 CBV 바인딩(`Bind`)
- **텍스처**: 텍스처 SRV 슬롯은 텍스처 로더 도입 이후 PHASE로 보류 (PHASE 05에서는 Constant 버퍼만 구현)

### ChalkScene
- **파일**: PHASE 05에 구현 (src/ChalkScene.h/.cpp)
- **담당**: 씬 객체 관리(Add/Remove/Get/순회), 조명(Directional light) Constant Buffer
- **주요 메서드**: `Add`/`Remove`/`Get` 객체 관리, 씬 순회, 조명 CB 업로드, 씬 렌더링 오케스트레이션

### ChalkBoard
- **파일**: PHASE 06에 생성 예정
- **담당**: 3D 보드 기하학, 표면 히트 테스팅

### ChalkStroke
- **파일**: PHASE 08에 생성 예정
- **담당**: 스트로크 데이터 구조, 점 리스트, 폭/색상

### ChalkToolSystem
- **파일**: PHASE 11에 생성 예정
- **담당**: 툴 선택 및 관리 (Chalk vs. Eraser)

### ChalkUndoRedo
- **파일**: PHASE 10에 생성 예정
- **담당**: 명령 이력, Undo/Redo 스택 관리

### ChalkSaveLoad
- **파일**: PHASE 12에 생성 예정
- **담당**: 그리기 데이터, 보드 상태, 카메라 저장/로드

## planned 파일 구성 (예정)
```
CHALKFORGE/
├ PROMPT_LOG.md          # PHASE 00에 생성된 프롬프트 이력
├ AGENTS.md              # PHASE 00에 생성된 에이전트 협업
├ PROJECT_PLAN.md        # PHASE 00에 생성된 phase 계획
├ ARCHITECTURE.md        # PHASE 00에 생성된 시스템 설계
├ ROADMAP.md           # PHASE 00-14 개발 로드맵
├ DECISIONS.md         # PHASE 00에 생성된 기술 결정사항
├ CHANGELOG.md         # PHASE 00에 생성된 함수 변경 기록
├ TEST_LOG.md          # PHASE 00에 생성된 빌드/테스트/오류 로그
├ TASKS.md             # PHASE 00에 생성된 작업 추적
├ LEARNING_LOG.md      # PHASE 00에 생성된 학습 개념
├ CODE_GUIDE.md        # PHASE 00에 생성된 코드 구조 가이드
├ CMakeLists.txt       # PHASE 01에 생성된 빌드 시스템
├ src/
│   ├── ChalkApp.h/.cpp          # PHASE 01/03 애플리케이션 진입 (ChalkApplication)
│   ├── ChalkWindow.h/.cpp       # PHASE 03 창 관리 (PHASE 02 WndProc 분리)
│   ├── ChalkLogger.h/.cpp       # PHASE 03 로깅
│   ├── ChalkError.h/.cpp        # PHASE 03 오류 처리
│   ├── ChalkConfig.h            # PHASE 03 구성 기본값
│   ├── ChalkGraphics.h/.cpp     # PHASE 03 D3D12 통합 (디바이스/큐/스왑 체인/RTV/PSO)
│   ├── ChalkFence.h/.cpp        # PHASE 04 GPU 동기화 펜스
│   ├── ChalkBarrier.h/.cpp      # PHASE 04 리소스 바리어 도우미
│   ├── ChalkResource.h/.cpp     # PHASE 04 GPU 리소스 래퍼/수명 추적
│   ├── ChalkMath.h/.cpp         # PHASE 03 최소 / PHASE 05 확장 (수학 유틸리티)
│   ├── ChalkCamera.h/.cpp       # PHASE 05 카메라 (뷰/프로젝션, WASD/궤도)
│   ├── ChalkMesh.h/.cpp         # PHASE 05 메시 (버텍스/인덱스/서브메쉬)
│   ├── ChalkMaterial.h/.cpp     # PHASE 05 머티리얼 (Constant 버퍼/텍스처)
│   ├── ChalkScene.h/.cpp        # PHASE 05 씬 객체 관리 + 조명 CB
│   ├── ChalkBoard.h/.cpp        # PHASE 06 3D 보드
│   ├── ChalkStroke.h/.cpp       # PHASE 08 스트로크 데이터
│   ├── ChalkToolSystem.h/.cpp   # PHASE 11 툴 관리
│   ├── ChalkUndoRedo.h/.cpp     # PHASE 10 Undo/Redo
│   ├── ChalkChalkTool.h/.cpp    # PHASE 08 칠 툴
│   ├── ChalkEraserTool.h/.cpp   # PHASE 09 지우개 툴
│   ├── ChalkSaveLoad.h/.cpp     # PHASE 12 저장/로드
│   └── ...                     # 단계가 진행됨에 따라 추가 파일
├ shaders/
│   ├── TriangleVS.hlsl    # PHASE 03 삼각형 버텍스 셰이더 (실행 시 컴파일)
│   ├── TrianglePS.hlsl    # PHASE 03 삼각형 픽셀 셰이더 (실행 시 컴파일)
│   ├── ChalkVS.hlsl       # PHASE 08+ 버텍스 셰이더
│   ├── ChalkPS.hlsl       # PHASE 08+ 픽셀 셰이더
│   ├── BoardVS.hlsl       # PHASE 06+ 보드 버텍스 셰이더
│   └── BoardPS.hlsl       # PHASE 06+ 보드 픽셀 셰이더
└ assets/                # 컴파일된 셰이더, 데이터 파일
```

## 코드 위치 및 관계에 대한 빠른 참조
### 초기화 흐름 (PHASE 03 Step F 기준)
1. `wWinMain()` → `ChalkApplication::Initialize()`
2. `ChalkApplication::Initialize()` → `ChalkWindow::Create()` (PHASE 02/03)
3. `ChalkApplication::Initialize()` → `ChalkGraphics::Initialize()` (PHASE 03)
4. `ChalkGraphics::Initialize()` → `CreateDeviceAndQueue()` → `CreateDXGIFactory2`/`D3D12CreateDevice` (PHASE 03)
5. `ChalkGraphics::Initialize()` → `CreateSwapChain()` → `CreateSwapChainForHwnd()` (PHASE 03)
6. `ChalkGraphics::Initialize()` → `CreateDescriptorHeaps()` (PHASE 03)
7. `ChalkGraphics::Initialize()` → `CreateRenderTargets()` (PHASE 03)
8. `ChalkGraphics::Initialize()` → `CreateCommandObjects()` (PHASE 03)
9. `ChalkGraphics::Initialize()` → `CreateFence()` (PHASE 03)
10. `ChalkGraphics::Initialize()` → `CreatePipeline()` → 루트 서명 + `D3DCompile` + PSO (PHASE 03)
11. `ChalkGraphics::Initialize()` → `CreateTriangleGeometry()` (PHASE 03)
12. `ChalkApplication::Initialize()` → `ChalkScene::Initialize()` (PHASE 05)
13. `ChalkScene::Initialize()` → `ChalkBoard::Initialize()` (PHASE 06)
14. `ChalkApplication::Initialize()` → 스트로크/툴 초기화 (PHASE 08+)

### 렌더링 흐름 (PHASE 03 Step F 기준)
1. 각 프레임: `ChalkApplication::Run()` → `ChalkWindow::ProcessMessages([&]{ graphics_->RenderFrame(); })`
2. 메시가 없으면 `onIdle`로 `ChalkGraphics::RenderFrame()` 호출
3. `ChalkGraphics::RenderFrame()` → `CommandAllocator::Reset()`
4. `ChalkGraphics::RenderFrame()` → `CommandList::Reset(allocator, PSO)`
5. `ChalkGraphics::PopulateCommandList()` → RTV 바리어(PRESENT→RT) → `OMSetRenderTargets` → `ClearRenderTargetView`
6. `ChalkGraphics::PopulateCommandList()` → `SetGraphicsRootSignature`/`SetPipelineState`/`IASetVertexBuffers`/`DrawInstanced(3,1,0,0)`
7. `ChalkGraphics::PopulateCommandList()` → RTV 바리어(RT→PRESENT) → `CommandList::Close()`
8. `ChalkGraphics::RenderFrame()` → `CommandQueue::ExecuteCommandLists()`
9. `ChalkGraphics::RenderFrame()` → `SwapChain::Present(1,0)` → `MoveToNextFrame()`

### 스트로크 렌더링 흐름 (planned, PHASE 08+)
1. CPU: `std::vector<Stroke> strokes`가 모든 스트로크 데이터 보유
2. CPU: 매 프레임, 스트로기 기하학에서 GPU 버텍스 버퍼 업데이트
3. CPU: `CommandList::IASetVertexBuffers()` → 스트로크 버텍스 버퍼 설정
4. CPU: `CommandList::DrawInstanced()` 또는 `DrawPrimitive()`당 스트로크
5. GPU: 버텍스 셰이더가 카메라 행렬을 사용하여 위치 변환 `camera.GetViewMatrix() * camera.GetProjectionMatrix() * world`
6. GPU: 픽셀 셰이더가 스트로크 색상 출력 (flat 또는 간단한 조명)
7. GPU: RTV를 통해 결과가 스왑 체인 뒷 버퍼에 병합

### 함수 cross-call 빠른 참조
| 함수/클래스 | 호출 | 호출됨 |
|----------------|-------|-----------|
| `ChalkApplication::Run()` | `ChalkGraphics::RenderFrame()`, `ChalkWindow::ProcessMessages()` | `WinMain` |
| `ChalkGraphics::RenderFrame()` | `CommandAllocator::Reset()`, `CommandList::Reset()`, `SwapChain::Present()` | `ChalkApplication::Run()` |
| `ChalkInputMapper::GetRayFromMouse()` | `ChalkCamera::GetViewProjectionMatrix()`, `ChalkBoard::RayIntersect()` | `ChalkWindow` (마우스 클릭) |
| `ChalkStroke::GenerateGeometry()` | `ChalkGraphics::CreateVertexBuffer()` (내부) | 스트로크 완료 |
| `ChalkUndoRedo::Undo()` | `ChalkDrawing::RemoveStroke()`, `ChalkBoard::ClearPoints()` | `ChalkApplication` (핫키) |
| `ChalkSaveLoad::Save()` | `ChalkSerializer::WriteStroke()`, `ChalkBoard::WriteConfig()`, `ChalkCamera::WriteState()` | 사용자 메뉴/핫키 |
| `ChalkToolSystem::SetActiveTool()` | `ChalkChalkTool::Activate()`, `ChalkEraserTool::Deactivate()` | `ChalkWindow` (키보드/마우스) |