CODE_GUIDE.md - CHALKFORGE 코드 가이드

## 엔트리 포인트
### WinMain (Win32 애플리케이션 시작점)
- **위치**: PHASE 01에 생성 (ChalkApp.cpp)
- **역할**: 애플리케이션 시작점; 창 클래스 등록, 메시 펌프, D3D12 초기화, 메인 루프
- **호출 순서**: 
  1. 윈도우 클래스 등록 (WNDCLASS)
  2. 창 생성 (CreateWindowEx) - "CHALKFORGE" 제목
  3. 메시 펌프 진입 (GetMessage/DispatchMessage)
  4. 각 반복: 메시 처리, 상태 업데이트, D3D12 프레임 렌더링
  5. 정리: 펜싱 대기, 리소스 처리, 모듈 종료

## planned 아키텍처의 핵심 클래스
### ChalkApplication
- **파일**: PHASE 01에 생성 예정
- **담당**: 메인 애플리케이션 오케스트레이션, 메시 루프, 수명 주기 관리
- **주요 메서드**: 
  - `Initialize()`: 창 생성, D3D12 디바이스 생성, 리소스 설정
  - `Run()`: 메인 메시 루프
  - `Finalize()`: 정리, 펜싱 대기, 처리
  - `OnResize()`: 스왑 체인과 리소스 크기 조정 처리

### ChalkGraphics
- **파일**: PHASE 03에 생성 예정
- **담당**: D3D12 디바이스, 명령 큐, 스왑 체인, 렌더 파이프라인 관리

### ChalkWindow
- **파일**: PHASE 02에 구현 완료 (ChalkApp.cpp 수정)
- **담당**: Win32 창 생성 및 메시 처리 (ChalkApp.cpp내 WndProc 구현)
- **주요 메서드**:
  - `Create(HINSTANCE, int, const char*)`: 창 생성 - ChalkApp.cpp 내 구현
  - `ProcessMessages()`: GetMessage/DispatchMessage 펌프 - ChalkApp.cpp 내 구현
  - `OnMessage(UINT, WPARAM, LPARAM)`: 메시 디스패치 - ChalkApp.cpp 내 구현
  - `SetResizeCallback()`: 크기 조정 이벤트 등록 - 향후 PHASE 07~08 추가 예정

### ChalkCamera
- **파일**: PHASE 05에 생성 예정
- **담당**: 카메라 위치, 방향, 뷰/프로젝션 행렬

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
│   ├── ChalkApp.h/.cpp          # PHASE 01 애플리케이션 진입
│   ├── ChalkDevice.h/.cpp       # PHASE 03 D3D12 디바이스
│   ├── ChalkCmdQueue.h/.cpp     # PHASE 03 명령 큐
│   ├── ChalkCmdList.h/.cpp      # PHASE 03 명령 목록
│   ├── ChalkRenderTarget.h/.cpp # PHASE 03 RTV/백 버퍼
│   ├── ChalkMath.h/.cpp         # PHASE 05 수학 유틸리티
│   ├── ChalkCamera.h/.cpp       # PHASE 05 카메라
│   ├── ChalkBoard.h/.cpp        # PHASE 06 3D 보드
│   ├── ChalkStroke.h/.cpp       # PHASE 08 스트로크 데이터
│   ├── ChalkToolSystem.h/.cpp   # PHASE 11 툴 관리
│   ├── ChalkUndoRedo.h/.cpp     # PHASE 10 Undo/Redo
│   ├── ChalkChalkTool.h/.cpp    # PHASE 08 칠 툴
│   ├── ChalkEraserTool.h/.cpp   # PHASE 09 지우개 툴
│   ├── ChalkSaveLoad.h/.cpp     # PHASE 12 저장/로드
│   └── ...                     # 단계가 진행됨에 따라 추가 파일
├ shaders/
│   ├── ChalkVS.hlsl       # PHASE 08+ 버텍스 셰이더
│   ├── ChalkPS.hlsl       # PHASE 08+ 픽셀 셰이더
│   ├── BoardVS.hlsl       # PHASE 06+ 보드 버텍스 셰이더
│   └── BoardPS.hlsl       # PHASE 06+ 보드 픽셀 셰이더
└ assets/                # 컴파일된 셰이더, 데이터 파일
```

## 코드 위치 및 관계에 대한 빠른 참조
### 초기화 흐름 (planned)
1. `WinMain()` → `ChalkApplication::Initialize()`
2. `ChalkApplication::Initialize()` → `ChalkWindow::Create()` (PHASE 02)
3. `ChalkApplication::Initialize()` → `ChalkGraphics::InitializeDevice()` (PHASE 03)
4. `ChalkGraphics::InitializeDevice()` → `CreateSwapChainForHwnd()` (PHASE 03)
5. `ChalkGraphics::InitializeDevice()` → `CreateCommandQueue()` (PHASE 03)
6. `ChalkGraphics::InitializeDevice()` → `CreateCommandAllocators()` (PHASE 03)
7. `ChalkGraphics::InitializeDevice()` → `CreateDescriptorHeaps()` (PHASE 03/04)
8. `ChalkGraphics::InitializeDevice()` → `CreateRootSignature()` (PHASE 04)
9. `ChalkGraphics::InitializeDevice()` → `CreatePipelineState()` (PHASE 04)
10. `ChalkApplication::Initialize()` → `ChalkScene::Initialize()` (PHASE 05)
11. `ChalkScene::Initialize()` → `ChalkBoard::Initialize()` (PHASE 06)
12. `ChalkApplication::Initialize()` → 스트로크/툴 초기화 (PHASE 08+)

### 렌더링 흐름 (planned)
1. 각 프레임: `ChalkApplication::Run()` 반복
2. `ChalkGraphics::RenderFrame()` 
3. `ChalkGraphics::RenderFrame()` → `CommandAllocator::Reset()`
4. `ChalkGraphics::RenderFrame()` → `CommandList::Reset(allocator, PSO)`
5. `ChalkGraphics::RenderFrame()` → 리소스 바리어 (백 버퍼, 깊이 전환)
6. `ChalkGraphics::RenderFrame()` → `Draw calls` (보드, 스트로크, UI)
7. `ChalkGraphics::RenderFrame()` → `CommandList::Close()`
8. `ChalkGraphics::RenderFrame()` → `CommandQueue::ExecuteCommandLists()`
9. `ChalkGraphics::RenderFrame()` → `SwapChain::Present()`

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