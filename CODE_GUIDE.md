CODE_GUIDE.md - CHALKFORGE 코드 가이드
======================================

## 엔트리 포인트
### WinMain (Win32 애플리케이션 시작점)
- **위치**: PHASE 01에 생성 예상 (ChalkApp.h/.cpp)
- **역할**: 애플리케이션 시작점; 창 클래스 등록, 메시 펌프, D3D12 초기화, 메인 루프
- **호출 순서**: 
  1. 윈도우 클래스 등록 (WNDCLASS)
  2. 창 생성 (CreateWindowEx)
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
- **주요 메서드**:
  - `InitializeDevice()`: 기능 수준 선택, 디바이스 생성
  - `CreateSwapChain()`: hwnd 연결 DXGI 스왑 체인 생성
  - `CreateCommandQueue()`: 직접 유형 명령 큐
  - `CreateCommandAllocators()`: 프레임당 할당자 관리
  - `CreateCommandLists()`: 명령 목록 생성 및 기록
  - `CreateDescriptorHeaps()`: RTV, DSV, CBV/SRV/UAV 힙
  - `CreatePipelineState()`: 다양한 렌더링 모드용 PSO 생성
  - `RenderFrame()`: 메인 렌더 함수 (명령 목록 기록, 드로우 콜, 스왑 체인 제시)

### ChalkWindow
- **파일**: PHASE 02에 생성 예정
- **담당**: Win32 창 생성 및 메시 처리
- **주요 메서드**:
  - `Create(HINSTANCE, int, const char*)`: 창 생성
  - `ProcessMessages()`: GetMessage/DispatchMessage 펌프
  - `OnMessage(UINT, WPARAM, LPARAM)`: 메시 디스패치
  - `SetResizeCallback()`: 크기 조정 이벤트 등록

### ChalkCamera
- **파일**: PHASE 05에 생성 예정
- **담당**: 카메라 위치, 방향, 뷰/프로젝션 행렬
- **주요 메서드**:
  - `Update(float deltaTime)`: 카메라 입력 처리
  - `GetViewMatrix()`: 뷰 행렬 계산
  - `GetProjectionMatrix()`: 투영 행렬 계산
  - `HandleMouse(float dx, float dy)`: 마우스 회전 제어
  - `HandleKeyboard(float deltaTime)`: WASD 이동

### ChalkBoard
- **파일**: PHASE 06에 생성 예정
- **담당**: 3D 보드 기하학, 표면 히트 테스팅
- **주요 메서드**:
  - `Initialize(float width, float height, float depth)`: 보드 기하학 생성
  - `GetWorldMatrix()`: 보드 변환 행렬
  - `RayIntersect(const XMVECTOR& origin, const XMVECTOR& direction)`: 평면 교차 테스트
  - `GetHitResult()`: 히트 포인트, 법선, 거리 반환 from ray 테스트

### ChalkStroke
- **파일**: PHASE 08에 생성 예정
- **담당**: 스트로크 데이터 구조, 점 리스트, 폭/색상
- **주요 메서드**:
  - `AddPoint(const XMVECTOR& point, float width, const XMVECTOR& color)`: 스트로크에 점 추가
  - `Complete()`: 스트로크 기하학 최종화
  - `GetPointCount()`: 샘플된 점의 개수
  - `GetPoint(int index)`: 인덱스 위치의 샘플된 점 가져오기
  - `GetWidth()`: 스트로크 폭
  - `GetColor()`: 스트로크 색상 (RGBA)
  - `GenerateGeometry()`: GPU 렌더링을 위한 버텍스 버퍼 생성

### ChalkToolSystem
- **파일**: PHASE 11에 생성 예정
- **담당**: 툴 선택 및 관리 (Chalk vs. Eraser)
- **주요 메서드**:
  - `SetActiveTool(ToolType type)`: 툴 간 전환
  - `GetActiveTool()`: 현재 활성화된 툴 유형 반환
  - `HandleInput()`: 입력 활성 툴로 라우팅
  - `HandleKeyShortcut(WPARAM wParam)`: 키보드 툴 전환

### ChalkUndoRedo
- **파일**: PHASE 10에 생성 예정
- **담당**: 명령 이력, Undo/Redo 스택 관리
- **주요 메서드**:
  - `Execute(Command command)`: 이력에 명령 추가, 동작 실행
  - `Undo()`: 명령 팝, 역전 실행
  - `Redo()`: Redo 스택에서 팝, 실행 전진
  - `GetUndoCount()`: undo 가능한 연산 개수
  - `GetRedoCount()`: redo 가능한 연산 개수

### ChalkSaveLoad
- **파일**: PHASE 12에 생성 예정
- **담당**: 그리기 데이터, 보드 상태, 카메라 저장/로드
- **주요 메서드**:
  - `Save(const char* filename)`: 스트로크 데이터 + 보드 + 바이너리 형식으로 카메라 저장
  - `Load(const char* filename)`: 상태 복원 및 역직렬화
  - `GetCurrentFormatVersion()`: 현재 형식 버전 번호 반환

## planned 파일 구성 (예정)
```
CHALKFORGE/
├── PROMPT_LOG.md          # PHASE 00에 생성된 프롬프트 이력
├── AGENTS.md              # PHASE 00에 생성된 에이전트 협업
├── PROJECT_PLAN.md        # PHASE 00에 생성된 phase 계획
├── ARCHITECTURE.md        # PHASE 00에 생성된 시스템 설계
├── ROADMAP.md           # PHASE 00-14 개발 로드맵
├── DECISIONS.md         # PHASE 00에 생성된 기술 결정사항
├── CHANGELOG.md         # PHASE 00에 생성된 함수 변경 기록
├── TEST_LOG.md          # PHASE 00에 생성된 빌드/테스트/오류 로그
├── TASKS.md             # PHASE 00에 생성된 작업 추적
├── LEARNING_LOG.md      # PHASE 00에 생성된 학습 개념
├── CODE_GUIDE.md        # PHASE 00에 생성된 코드 구조 가이드
├── CMakeLists.txt       # PHASE 01에 생성된 빌드 시스템
├── src/
│   ├── ChalkApp.h/.cpp          # PHASE 01 애플리케이션 진입
│   ├── ChalkWindow.h/.cpp       # PHASE 02 창 관리
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
├── shaders/
│   ├── ChalkVS.hlsl       # PHASE 08+ 버텍스 셰이더
│   ├── ChalkPS.hlsl       # PHASE 08+ 픽셀 셰이더
│   ├── BoardVS.hlsl       # PHASE 06+ 보드 버텍스 셰이더
│   └── BoardPS.hlsl       # PHASE 06+ 보드 픽셀 셰이더
└── assets/                # 컴파일된 셰이더, 데이터 파일
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

## 함수 cross-call 빠른 참조
| 함수/클래스 | 호출 | 호출됨 |
|----------------|-------|-----------|
| `ChalkApplication::Run()` | `ChalkGraphics::RenderFrame()`, `ChalkWindow::ProcessMessages()` | `WinMain` |
| `ChalkGraphics::RenderFrame()` | `CommandAllocator::Reset()`, `CommandList::Reset()`, `SwapChain::Present()` | `ChalkApplication::Run()` |
| `ChalkInputMapper::GetRayFromMouse()` | `ChalkCamera::GetViewProjectionMatrix()`, `ChalkBoard::RayIntersect()` | `ChalkWindow` (마우스 클릭) |
| `ChalkStroke::GenerateGeometry()` | `ChalkGraphics::CreateVertexBuffer()` (내부) | 스트로크 완료 |
| `ChalkUndoRedo::Undo()` | `ChalkDrawing::RemoveStroke()`, `ChalkBoard::ClearPoints()` | `ChalkApplication` (핫키) |
| `ChalkSaveLoad::Save()` | `ChalkSerializer::WriteStroke()`, `ChalkBoard::WriteConfig()`, `ChalkCamera::WriteState()` | 사용자 메뉴/핫키 |
| `ChalkToolSystem::SetActiveTool()` | `ChalkChalkTool::Activate()`, `ChalkEraserTool::Deactivate()` | `ChalkWindow` (키보드/마우스) |