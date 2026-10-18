CHANGELOG.md - CHALKFORGE Change Log
=====================================

No changes yet. This file will track all functional additions, modifications, and removals.

Format:
### [Unreleased]
- Added
- Changed
- Fixed

### [PHASE 00 COMPLETE] - 2026-10-12
- Created project structure and documentation framework
- Environment audit: MSVC 14.44.35207, Direct3D 12, Windows Kits 10.x confirmed
- Architecture and roadmap documents created
- PROMPT_LOG.md, AGENTS.md, PROJECT_PLAN.md, ARCHITECTURE.md, ROADMAP.md, DECISIONS.md initialized
- Environment validation for PHASE 01 implementation
- CMakeLists.txt 기본 프로젝트 구조 생성 확인
- ChalkApp.cpp 기본 애플리케이션 입력점 생성 확인
- Visual Studio 2022 프로젝트 생성 및 빌드 검증

### [PHASE 01 DOCUMENTATION] - 2026-10-12
- PHASE_01.md 생성: 목표, 파일, CMake 빌드 및 실행 검증 결과
- PHASE_02.md 생성: 현재 상태 미구현 및 향후 목표 문서화
- AGENTS.md 업데이트: 작업 전 후 규칙 추가
- PROJECT_PLAN.md 업데이트: PHASE 01 진행 상태 및 완료된 작업 기록
- CHANGELOG.md_self 문서화 체계 확립

### [PHASE 02 IMPLEMENTATION] - 2026-10-12
- Win32 API를 이용한 Windows 창 생성 구현
- WinMain 진입점 및 WndProc 메시지 처리 함수 구현
- 메시지 루프 구성 및 WM_DESTROY 처리 통해 정상 종료 지원
- CMakeLists.txt /subsystem:windows 플래그 적용, Windows GUI 애플리케이션 설정
- "CHALKFORGE" 제목의 창 성공적 표시 및 닫기 검증

### [PHASE 03 IMPLEMENTATION] - 2026-10-19
- Added
  - ChalkConfig.h: 창/스왑 체인/로그 관련 상수 (kWindowTitle, kDefaultWindowWidth/Height, kFrameCount 등)
  - ChalkLogger (.h/.cpp): 레벨별 로깅, OutputDebugStringA + 파일 로그, InfoF/ErrorF 등 포맷 헬퍼
  - ChalkError (.h/.cpp): ChalkFormatHResult, ChalkException, FAIL/THROW_IF_FAILED 매크로
  - ChalkMath.h: DirectXMath 별칭, kPi, DegreesToRadians/RadiansToDegrees, Identity
  - ChalkApp.h: ChalkApplication 클래스 및 wWinMain 진입점 선언
  - ChalkWindow (.h/.cpp): CreateWindowExW 기반 창 생성, ProcessMessages(onIdle) 메시지 처리
  - ChalkGraphics (.h/.cpp): D3D12 디바이스/커맨드 큐/스왑 체인/기술 힙/루트 서명/PSO/정점 버퍼/펜스 동기화/리사이즈
  - shaders/TriangleVS.hlsl, shaders/TrianglePS.hlsl: 삼각형 렌더링 셰이더
- Changed
  - ChalkApp.cpp: ChalkApplication으로 리팩터링, 그래픽스 연계 및 렌더 루프 구성, Per-Monitor V2 DPI 적용
  - CMakeLists.txt: ChalkGraphics.cpp 추가, d3d12/dxgi/d3dcompiler 링크, NOMINMAX/UNICODE, /utf-8, /subsystem:windows, Debug CHALK_DEBUG_LAYER, 셰이더 POST_BUILD 복사
- Fixed
  - C4244(wide→narrow) 경고: ChalkGraphics.cpp 내부 WideToUtf8 헬퍼로 셰이더 경로 변환
- Removed
  - build/ 디렉터리 Git 추적 해제 (git rm -r --cached build, .gitignore에 build/ 추가)

### [PHASE 04 IMPLEMENTATION] - 2026-10-19
- Added
  - ChalkFence (.h/.cpp): 펜스/이벤트/per-frame 값 캡슐화 (Initialize/Shutdown/IsInitialized/Signal/WaitForValue/WaitForFrame/WaitForGpu/CompletedValue/FrameValue/SetFrameValue)
  - ChalkBarrier (.h/.cpp): `Transition(list, resource, before, after)` 리소스 상태 전이 도우미
  - ChalkResource (.h/.cpp): ID3D12Resource 래퍼(state/이름/수명 로깅) + CreateDepthBuffer 도우미
- Changed
  - ChalkConfig.h: `kSwapChainBufferCount` 2 → 3 (트리플 버퍼링)
  - ChalkGraphics.h/.cpp: fence 로직을 `ChalkFence`로 이관, 인라인 barrier를 `ChalkBarrier::Transition`으로 대체, 백 버퍼별 깊이 리소스(`depthStencils_[kFrameCount]`) + DSV 힙(`dsvHeap_`, `dsvDescriptorSize_`) 추가, `CreateDepthStencilBuffer(int, int)`, PSO depth state(`DepthEnable=TRUE`/`DepthWriteMask=ALL`/`DepthFunc=LESS`/DSVFormat=D32_FLOAT), `OMSetRenderTargets`에 DSV 연결 및 `ClearDepthStencilView`
  - ChalkGraphics.cpp `OnResize`: 깊이 리소스 Reset 후 재생성, 재생성 결과 검사·오류 로깅 하드닝
  - ChalkGraphics.cpp `ReleaseResources`: `depthStencils_`/`dsvHeap_` 해제 및 `dsvDescriptorSize_` 초기화
  - CMakeLists.txt: `ChalkFence.cpp`, `ChalkBarrier.cpp`, `ChalkResource.cpp` 추가
- Verified
  - Debug/Release 빌드 성공
  - 실행 로그 확인: 깊이 버퍼 3개 생성(1280x720) 및 정상 종료, ERROR 레벨 로그 없음
  - 사용자 동작 확인 완료(삼각형 렌더링 유지/깊이 버퍼/리사이즈) → PHASE 04 완료

### [PHASE 05 STEP B] - 2026-10-19
- Added
  - ChalkMath.cpp: DirectXMath 기반 수학 연산 구현 (Multiply/Transpose/Inverse, Translation/Scaling, RotationX/Y/Z/RotationYawPitchRoll, LookAtRH, PerspectiveFovRH, Length/Dot/Cross/Normalize, ToUploadMatrix)
- Changed
  - ChalkMath.h: Matrix4x4(전치/곱셈/역행렬/변환)·벡터·투영·업로드 헬퍼 선언 추가 (전부 `noexcept`)
  - CMakeLists.txt: `ChalkMath.cpp` 소스 추가
- Verified
  - Debug/Release 빌드 성공 (`ChalkMath.cpp` 컴파일 및 링크 확인)
  - 미검증: 런타임 동작 (호출부 없음 → Step C 이후 검증 예정)

### [PHASE 05 STEP C] - 2026-10-19
- Added
  - ChalkCamera (.h/.cpp): 위치/타겟/업 기반 카메라. `SetPosition`/`SetTarget`/`SetUp`, `SetPerspective`(fovY/aspect/near/far)·`SetFovY`/`SetAspectRatio`·`SetClipPlanes`, 접근자(`Position`/`Target`/`Up`/`FovYRadians`/`AspectRatio`/`NearZ`/`FarZ`), `GetViewMatrix`(LookAtRH)/`GetProjectionMatrix`(PerspectiveFovRH)/`GetViewProjectionMatrix`, `MoveForward`/`MoveRight`(pan)/`Orbit(yaw,pitch)`
- Changed
  - CMakeLists.txt: `ChalkCamera.cpp` 소스 추가
- Fixed
  - ChalkCamera.cpp: `DirectX::XMVECTOR`의 `operator+`/`operator-` 미정의(C2676)·`XMStoreFloat3` 인수 오류(C2660) → 이 빌드 구성에서 XMVECTOR 산술 연산자 비활성 → `XMVectorAdd`/`XMVectorSubtract` 명시적 함수로 교체(`ChalkMath.cpp`와 동일 스타일)
- Verified
  - Debug/Release 빌드 성공 (`ChalkCamera.cpp` 컴파일 및 링크 확인)
  - 미검증: 런타임 동작 (호출부 없음 → Step F/G에서 검증 예정)

### [PHASE 05 STEP D] - 2026-10-19
- Added
  - ChalkMesh (.h/.cpp): 정점/서브메쉬 기반 메시 관리. `ChalkVertex`(position/normal/texcoord/tangent), `ChalkSubmesh`(indexOffset/indexCount/materialIndex), `ChalkMesh`(`Create`/`Reset`/`IsValid`/`VertexCount`/`IndexCount`/`SubmeshCount`/`GetSubmesh`/`VertexBufferView`/`IndexBufferView`/`Draw`/`DrawSubmesh`)
- Changed
  - CMakeLists.txt: `ChalkMesh.cpp` 소스 추가
- Verified
  - Debug/Release 빌드 성공 (`ChalkMesh.cpp` 컴파일 및 링크 확인, shaders POST_BUILD 복사 정상)
  - 미검증: 런타임 동작 (호출부 없음 → Step G에서 검증 예정)

### [PHASE 05 STEP E] - 2026-10-19
- Added
  - ChalkMaterial (.h/.cpp): 머티리얼 Constant 버퍼 관리. `ChalkMaterialConstants`(baseColor/emissive `XMFLOAT4`, roughness/metallic/specularPower/opacity `float` = 48바이트, 전부 16-byte 정렬), `ChalkMaterial`(`Create`/`Reset`/`IsValid`/`Constants`/`SetBaseColor`/`SetEmissive`/`SetRoughness`/`SetMetallic`/`SetSpecularPower`/`SetOpacity`/`Upload`/`Bind`)
  - Constant 버퍼는 UPLOAD 힙에 `D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT`(256바이트) 크기로 생성, `Map`/`memcpy`/`Unmap` 업로드, `Bind(rootParameterIndex)`는 `SetGraphicsRootConstantBufferView`로 루트 CBV 바인딩
- Changed
  - CMakeLists.txt: `ChalkMaterial.cpp` 소스 추가
- Verified
  - Debug/Release 빌드 성공 (`ChalkMaterial.cpp` 컴파일 및 링크 확인, shaders POST_BUILD 복사 정상)
  - 미검증: 런타임 동작 (호출부 없음 → Step F에서 루트 서명 CBV 슬롯 추가 후 Step G에서 검증 예정)

### [PHASE 05 STEP F] - 2026-10-19
- Added
  - ChalkScene (.h/.cpp): 씬 객체 관리. `ChalkSceneObject`(name + 월드 행렬 `XMFLOAT4X4` + 비소유 `ChalkMesh*`/`ChalkMaterial*`), `ChalkScene`(`Add`/`Remove`/`Clear`/`Get`(이름·인덱스)/`Count`/`Objects`/`Light`/`BuildLightConstants`), `ChalkDirectionalLight`(direction/color/intensity), `ChalkDirectionalLightConstants`(48바이트). D3D12 의존 없음(ChalkMesh/ChalkMaterial 전방 선언)
  - ChalkGraphics: 씬 통합 렌더 연결. `CreateSceneResources`(큐브 24정점/36인덱스 메시 + 머티리얼 2개 + 카메라·씬 구성), `CreateUploadBuffer(UINT64, name, outResource)`(UPLOAD 힙 업로드 버퍼 헬퍼)
  - Transform/Material/Light Constant Buffer: Transform b0(per-object `{ world, worldViewProj }` = 128바이트), Material b1(48바이트), Light b2(per-frame 48바이트). object CB = `kFrameCount * kMaxSceneObjects * 256`, light CB = `kFrameCount * 256`
- Changed
  - ChalkGraphics.h/.cpp: `vertexBuffer_`/`vertexBufferView_` 제거 → `objectConstantBuffer_`/`lightConstantBuffer_` + `ChalkCamera camera_`/`ChalkScene scene_`/`ChalkMesh cubeMesh_`/`ChalkMaterial materials_[kMaterialCount]` 추가. 루트 서명에 CBV 3슬롯(b0/b1/b2, `D3D12_SHADER_VISIBILITY_ALL`, `RegisterSpace=0`) 추가. PSO 입력 레이아웃을 하드코딩 삼각형에서 `ChalkVertex`(POSITION/NORMAL/TEXCOORD/TANGENT, stride 44)로 교체, `CullMode=NONE`
  - ChalkGraphics.cpp `PopulateCommandList`: light CB(프레임별) + 씬 순회(per-object Transform CB, `ToUploadMatrix`로 전치 업로드, material `Bind`/mesh `Draw`)로 재작성. `OnResize`에 `camera_.SetAspectRatio` 추가. `ReleaseResources`/`Finalize`에서 CB·메시·머티리얼·씬 해제
  - shaders/TriangleVS.hlsl, TrianglePS.hlsl: transform(b0)/material(b1)/light(b2) cbuffer 및 Lambert+ambient(0.15)+emissive 조명, `mul(rowVector, matrix)` 형태로 갱신
  - CMakeLists.txt: `ChalkScene.cpp` 소스 추가
- Verified
  - Debug/Release 빌드 성공 (`ChalkScene.cpp` 컴파일 및 링크, shaders POST_BUILD 복사 정상)
  - 실행 스모크 테스트 통과: `build/Debug/CHALKFORGE.exe` 4초 실행 후 정상 종료(ExitCode=0), 로그 오류 없음(메시 생성 24v/36i, 머티리얼 2개, 초기화/해제 로그)
  - 미검증: 육안 렌더(씬 큐브 2개 + 방향성 조명) 사용자 확인 대기, D3D12 debug layer validation 메시지 미캡처, 카메라 이동(WASD/궤도) 입력 미구현

### [PHASE 05 STEP G] - 2026-10-19
- Verified
  - Debug/Release 빌드 성공(Step F 산출물 기준, 문서화 단계로 코드 변경 없음)
  - 실행 스모크 테스트 통과(무크래시 종료, 초기화/해제 로그 오류 없음)
  - 사용자 육안 확인 완료: 씬 큐브 2개(CenterCube/OffsetCube) 정상 표시("큐브 두 개 나온다")
- Result
  - PHASE 05 렌더 통합(ChalkMath/ChalkCamera/ChalkMesh/ChalkMaterial/ChalkScene + ChalkGraphics) 검증 완료
- Remaining (후속 작업)
  - 카메라 이동(WASD/마우스 궤도) 입력 연결 미구현(정적 카메라)
  - D3D12 debug layer validation 메시지 파일 로그 미캡처

### [PHASE 05 STEP H] - 2026-10-19
- Added
  - ChalkWindow: 키보드/마우스 입력 상태 추적 — `enum MouseButton`(`kMouseLeft=0`/`kMouseRight=1`/`kMouseMiddle=2`/`kMouseButtonCount=3`), 접근자 `IsKeyDown(int)`/`IsMouseButtonDown(int)`/`MousePosition()`/`ConsumeMouseDelta(int&,int&)`(consume+reset), private `ResetInputState()`, 멤버 `keyDown_`/`mouseDown_`/`mousePosition_`/`mouseDeltaX_`/`mouseDeltaY_`/`mouseTracked_`
  - ChalkWindow: `OnMessage`에 `WM_KEYDOWN`/`WM_KEYUP`, `WM_SYSKEYDOWN`/`WM_SYSKEYUP`, `WM_MOUSEMOVE`(누적 델타, 첫 이동 baseline 스킵), `WM_LBUTTON/RBUTTON/MBUTTONDOWN/UP`(`SetCapture`/`ReleaseCapture`), `WM_SETFOCUS`/`WM_KILLFOCUS` 처리 추가
  - ChalkApp: QPC(QueryPerformanceCounter) 기반 `ComputeDeltaSeconds()`, 입력 폴링 → 카메라 구동 `UpdateCamera(float)`, idle 콜백 연결
- Changed
  - ChalkConfig.h: `kCameraMoveSpeed = 4.0f`(units/sec), `kCameraOrbitSensitivity = 0.006f`(radians/pixel), `kMaxFrameDeltaSeconds = 0.1f`(dt 스파이크 clamp) 상수 추가
  - ChalkGraphics.h: `ChalkCamera& Camera() noexcept` + const 오버로드 추가(camera_는 private 유지)
  - ChalkApp.h/.cpp: 타이밍 멤버(`performanceFrequency_`/`lastFrameCounter_`/`hasLastFrameCounter_`) 추가, `#include "ChalkCamera.h"`, `Run()`에서 `QueryPerformanceFrequency`/`QueryPerformanceCounter` 초기화 및 idle 콜백에서 `ComputeDeltaSeconds()` → `UpdateCamera(dt)` → `graphics_->RenderFrame()` 호출, `ComputeDeltaSeconds`/`UpdateCamera` 구현
- Input Mapping
  - W/S/A/D → `camera.MoveForward`/`camera.MoveRight`(이동량 = `kCameraMoveSpeed * dt`), 마우스 좌클릭 드래그 → `camera.Orbit(yaw, pitch)`(`yaw = -dx*sens`, `pitch = -dy*sens`)
- Verified
  - Debug/Release 빌드 성공
  - Release 실행 스모크 테스트 통과(4초 이상 실행 유지, D3D12 초기화 로그 정상, 무크래시)
- Remaining (미검증)
  - 입력 동작(WASD 이동/좌클릭 드래그 궤도) 사용자 육안 확인 대기(합성 입력 주입 불가로 자동 검증 불가)

### [Future Phases]
- To be populated as development progresses.