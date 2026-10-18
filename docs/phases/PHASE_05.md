PHASE 05 — 카메라, 수학, 메시 및 씬 시스템

**목표**: 완전한 수학, 카메라, 메시 및 씬 인프라스트럭처를 구현한다. DirectXMath 기반 행렬/벡터 연산, 뷰·투영 카메라, 버텍스/인덱스 버퍼를 가진 메시, Constant 버퍼 머티리얼, 씬 객체 관리, 방향성 조명을 구현하고 렌더 파이프라인에 통합한다.

**구현 완료 상태**: Step A(문서 정합화) 완료. Step B(ChalkMath 확장) 완료(Debug/Release 빌드 성공). Step C(ChalkCamera) 완료(Debug/Release 빌드 성공). Step D(ChalkMesh) 완료(Debug/Release 빌드 성공). Step E(ChalkMaterial) 완료(Debug/Release 빌드 성공). Step F(ChalkScene + 조명 + ChalkGraphics 전체 렌더 연결) 완료(Debug/Release 빌드 성공, 실행 스모크 테스트 통과). Step G(검증 + 문서화) 완료 — 사용자 육안 확인 완료(씬 큐브 2개 + 방향성 조명 정상 표시). Step H(카메라 입력 연결) 구현 완료(Debug/Release 빌드 성공, 실행 스모크 테스트 통과) — 입력 동작(WASD/좌클릭 드래그)은 사용자 육안 확인 대기. **PHASE 05 카메라 입력 연결 코드 구현 완료(입력 동작 육안 확인 대기).**

**내부 검증 단계** (각 항목을 개별 검증):
1. 수학 — DirectXMath 기반 Matrix4x4 연산(전치/곱셈/역행렬/투영)이 정상 동작
2. 카메라 — 위치/타겟/업 기반 뷰 행렬과 시야각/종횡비/근·원 플레인 기반 투영 행렬 계산 정상
3. 메시 — 버텍스/인덱스 버퍼 생성 및 서브메쉬 관리 정상
4. 머티리얼 — Constant 버퍼(색상/조명 파라미터) 생성 및 슬롯 바인딩 정상
5. 씬 — 씬 객체 Add/Remove/Get/순회 정상
6. 조명 — 방향성 조명 Constant 버퍼 전달 및 셰이더 반영 정상
7. 행렬 컨벤션 — DirectXMath(row-major) → HLSL cbuffer(column-major) 정합(업로드 시 `XMMatrixTranspose`)

**핵심 설계 결정** (DECISIONS.md 참조):
- 좌표/행렬 컨벤션: DirectXMath 기본 row-major를 유지하고, HLSL Constant 버퍼 업로드 시 `XMMatrixTranspose`로 column-major 변환한다(표준 Direct3D 12 샘플 방식). 대안(`row_major` 키워드)은 거부한다.
- 수학 계층: `ChalkMath`는 DirectXMath 별칭/헬퍼 위에 Matrix4x4 연산을 추가하고 필요 시 `ChalkMath.cpp`로 구현을 분리한다.
- 카메라/씬 통합: 기존 `ChalkGraphics` 렌더 파이프라인에 카메라 행렬·씬 순회를 연결하되, PHASE 04의 헬퍼 분리 원칙(과도한 추상화 회피)을 유지한다.
- XMVECTOR 산술: 이 빌드 구성에서는 DirectXMath `XMVECTOR`의 `operator+`/`operator-`가 비활성(C2676)이므로 `XMVectorAdd`/`XMVectorSubtract`/`XMVectorScale` 등 명시적 함수만 사용한다(`ChalkMath.cpp`와 동일 스타일).

**작업 단계 및 변경 예정 파일**:

1. **Step A — 문서 정합화** (완료)
   - 수정: `PROJECT_PLAN.md`, `ROADMAP.md`, `TASKS.md`, `CODE_GUIDE.md`, `ARCHITECTURE.md`, `DECISIONS.md`
   - 신규: `docs/phases/PHASE_05.md` (본 문서)

2. **Step B — `ChalkMath` 확장** (완료)
   - 수정: `src/ChalkMath.h`; 신규: `src/ChalkMath.cpp`, `CMakeLists.txt`에 소스 추가
   - Matrix4x4 연산(전치/곱셈/역행렬), 투영 행렬(원근), 벡터 연산 확장
   - 구현 내용:
     - `ChalkMath.h`: `Multiply`/`Transpose`/`Inverse`, `Translation`(x,y,z / XMFLOAT3), `Scaling`(균등/3축), `RotationX`/`RotationY`/`RotationZ`/`RotationYawPitchRoll`, `LookAtRH`, `PerspectiveFovRH`, `Length`/`Dot`/`Cross`/`Normalize`(XMFLOAT3), `ToUploadMatrix`(row-major→cbuffer column-major) 선언 추가
     - `ChalkMath.cpp`: 위 선언을 DirectXMath(`XMMatrix*`/`XMVector*`)로 구현, 전부 `noexcept`
   - 검증: Debug/Release 빌드 성공 (`ChalkMath.cpp` 컴파일 및 링크 확인)
   - 미검증: 런타임 동작 (아직 호출부 없음 → Step C 이후 실제 사용 시 검증 예정)

3. **Step C — `ChalkCamera`** (완료)
   - 신규: `src/ChalkCamera.h`/`.cpp`, `CMakeLists.txt`에 소스 추가
   - 위치/타겟/업, 뷰·투영 행렬 계산, 이동(WASD/pan)·마우스 궤도 제어
   - 구현 내용:
     - `ChalkCamera.h`: `SetPosition`/`SetTarget`/`SetUp`, `SetPerspective`(fovY/aspect/near/far)·`SetFovY`/`SetAspectRatio`(인라인)·`SetClipPlanes`, `Position()`/`Target()`/`Up()`/`FovYRadians()`/`AspectRatio()`/`NearZ()`/`FarZ()`, `GetViewMatrix()`/`GetProjectionMatrix()`/`GetViewProjectionMatrix()`, `MoveForward`/`MoveRight`/`Orbit(yaw,pitch)`
     - 기본값: position (0,0,5), target 원점, up (0,1,0), fovY 60°(라디안), aspect 16/9, near 0.1, far 1000
     - `GetViewMatrix`→`ChalkMath::LookAtRH`, `GetProjectionMatrix`→`ChalkMath::PerspectiveFovRH`, `GetViewProjectionMatrix`→`ChalkMath::Multiply(view, projection)`. `MoveForward`/`MoveRight`는 position·target 동일 delta 평행이동(pan), `Orbit`은 target 기준 offset 회전
   - 구현 중 수정: 최초 빌드에서 `DirectX::XMVECTOR`의 `operator+`/`operator-` 미정의(C2676)·`XMStoreFloat3` 인수 오류(C2660) 발생 → 이 빌드 구성에서 XMVECTOR 산술 연산자 비활성 → `XMVectorAdd`/`XMVectorSubtract` 명시적 함수로 교체(`ChalkMath.cpp`와 동일 스타일)
   - 검증: Debug/Release 빌드 성공 (`ChalkCamera.cpp` 컴파일 및 링크 확인)
   - 미검증: 런타임 동작 (아직 호출부 없음 → Step F/G에서 실제 사용 시 검증 예정)

4. **Step D — `ChalkMesh`** (완료)
   - 신규: `src/ChalkMesh.h`/`.cpp`, `CMakeLists.txt` 소스 추가
   - 버텍스/인덱스 버퍼, 서브메쉬 관리
   - 구현 내용:
     - `ChalkMesh.h`: `ChalkVertex`(position/normal/texcoord/tangent = 44 byte, float만), `ChalkSubmesh`(`indexOffset`/`indexCount`/`materialIndex`), `ChalkMesh` 클래스(`Create`/`Reset`/`IsValid`/`VertexCount`/`IndexCount`/`SubmeshCount`/`GetSubmesh`/`VertexBufferView`/`IndexBufferView`/`Draw`/`DrawSubmesh`). 복사/대입 `= delete`, 버퍼는 `ComPtr<ID3D12Resource>` 소유.
     - `ChalkMesh.cpp`: 인자 검증 실패 시 로그 후 `false`, `ChalkGraphics::CreateTriangleGeometry`와 동일한 UPLOAD 힙/`CreateCommittedResource`/`Map`-`memcpy`-`Unmap` 패턴으로 버텍스·인덱스 버퍼 생성, `SetName`(name, name+L" Indices"), 뷰 구성(Stride=44, `DXGI_FORMAT_R32_UINT`), 서브메쉬 `assign`. `Draw`/`DrawSubmesh`는 `IASetVertexBuffers`/`IASetIndexBuffer` 후 `DrawIndexedInstanced`.
   - 검증: Debug/Release 빌드 성공 (`ChalkMesh.cpp` 컴파일 및 링크 확인)
   - 미검증: 런타임 동작 (호출부 없음 → Step F/G에서 검증 예정)

5. **Step E — `ChalkMaterial`** (완료)
   - 신규: `src/ChalkMaterial.h`/`.cpp`, `CMakeLists.txt` 추가
   - Constant 버퍼(머티리얼 색상/조명 파라미터)만 구현. 텍스처 SRV 슬롯은 텍스처 로더 도입 이후 PHASE로 보류(ARCHITECTURE.md 146-150, CODE_GUIDE.md 90-93 반영).
   - 구현 내용:
     - `ChalkMaterial.h`: `ChalkMaterialConstants`(baseColor/emissive: `XMFLOAT4`, roughness/metallic/specularPower/opacity: float → 48 byte, 전부 16-byte 정렬), `ChalkMaterial` 클래스(`Create`/`Reset`/`IsValid`/`Constants`/`SetBaseColor`/`SetEmissive`/`SetRoughness`/`SetMetallic`/`SetSpecularPower`/`SetOpacity`/`Upload`/`Bind`). 복사/대입 `= delete`, CB는 `ComPtr<ID3D12Resource>` 소유.
     - `ChalkMaterial.cpp`: `kConstantBufferSize = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT`(256 byte) UPLOAD 힙 버퍼, `Map`-`memcpy`-`Unmap` 업로드(ChalkMesh와 동일 패턴), `Bind`는 `SetGraphicsRootConstantBufferView(rootParameterIndex, GPU 가상 주소)`(루트 CBV, 디스크립터 힙 불필요). 호출자가 루트 파라미터 인덱스를 전달.
   - 참고: 현재 루트 서명에 CBV 슬롯이 없으므로 호출부는 없음(Step F에서 루트 서명 CBV 슬롯 추가 시 연결). Step E는 빌드 검증만 수행.
   - 검증: Debug/Release 빌드 성공 (`ChalkMaterial.cpp` 컴파일 및 링크 확인)
   - 미검증: 런타임 동작 (호출부 없음 → Step F/G에서 검증 예정)

6. **Step F — `ChalkScene` + 조명 + `ChalkGraphics` 전체 렌더 연결** (완료)
   - 신규: `src/ChalkScene.h`/`.cpp`, `CMakeLists.txt` 추가
   - 수정: `src/ChalkGraphics.h`/`.cpp`, `shaders/TriangleVS.hlsl`/`TrianglePS.hlsl`
   - 씬 객체 Add/Remove/Get/순회, 방향성 조명 Constant 버퍼, 카메라 행렬·씬 순회를 실제 렌더 패스에 연결
   - 구현 내용:
     - `ChalkScene.h`/`.cpp`: `ChalkSceneObject`(name + `world`(`XMFLOAT4X4`, 단위행렬 기본값) + 비소유 `ChalkMesh*`/`ChalkMaterial*`), `ChalkScene`(`Add`(반환 `ChalkSceneObject&`)/`Remove`/`Clear`/`Get`(이름/인덱스)/`Count`/`Objects`/`Light`/`BuildLightConstants`). `ChalkDirectionalLight`(direction/color/intensity), `ChalkDirectionalLightConstants`(48바이트, HLSL b2와 정합). D3D12 비의존(전방 선언만).
     - `ChalkGraphics.h`: `kMaterialCount=2`/`kMaxSceneObjects=16` 상수 추가, `camera_`/`scene_`/`cubeMesh_`/`materials_[]`/`objectConstantBuffer_`/`lightConstantBuffer_` 멤버 추가, `CreateSceneResources`/`CreateUploadBuffer` 선언, `vertexBuffer_`/`vertexBufferView_` 제거.
     - `ChalkGraphics.cpp` `CreateSceneResources`: 하드코딩 삼각형 대신 씬 관리 큐브(6면 × 4정점 = 24정점, 면별 외향 normal/tangent, uv)와 36인덱스 생성(`ChalkMesh`), 머티리얼 2개 생성·업로드(회색/적색), 씬에 `CenterCube`/`OffsetCube`(translation 2.2,0,0) 추가, 카메라 위치/타겟 설정, 방향성 조명 설정. per-object Transform CB(프레임×최대오브젝트 슬라이스, `kFrameCount*kMaxSceneObjects*256`)와 per-frame Light CB(`kFrameCount*256`)를 `CreateUploadBuffer`(UPLOAD 힙)로 생성.
     - `ChalkGraphics.cpp` `PopulateCommandList`: 씬 순회하며 오브젝트별 Transform CB(`world` + `worldViewProj = Multiply(world, viewProjection)`)를 Map/memcpy/Unmap 후 루트 CBV(b0) 바인딩, 머티리얼 CB(b1) 바인딩, 메시 `Draw`. 프레임별 Light CB(b2) 갱신. CPU row-major 행렬은 업로드 직전 `ChalkMath::ToUploadMatrix`(전치) 적용.
     - 루트 서명: CBV 3개(b0 transform / b1 material / b2 light), `ShaderVisibility=ALL`. PSO 입력 레이아웃을 `ChalkVertex`(stride 44)로 교체. `OnResize`에서 `camera_.SetAspectRatio` 갱신. `ReleaseResources`에서 CB/메시/머티리얼/씬 해제.
     - `shaders/TriangleVS.hlsl`/`TrianglePS.hlsl`: transform(b0)/material(b1)/light(b2) cbuffer, `mul(rowVector, matrix)` 방식, Lambert + 앰비언트 + emissive 조명.
   - 검증: Debug/Release 빌드 성공. 실행 스모크 테스트 통과(무크래시 종료, 초기화/해제 로그 정상).
   - 미검증: 육안 렌더 결과(사용자 확인 대기). D3D12 debug layer validation 메시지는 OutputDebugString으로 출력되어 로그에 남지 않음 → 로그상 오류 없음은 스모크 테스트 수준. 카메라 이동(WASD)·궤도 제어는 UI 입력 연결이 없어 미구현(후속 작업).

7. **Step G — 검증 + 문서화** (완료)
   - 수정: `CHANGELOG.md`, `TEST_LOG.md`, `PROMPT_LOG.md`, `TASKS.md`, 본 문서
   - Debug/Release 빌드 성공, 실행 로그(오류 없음) 확인, 사용자 육안 확인
   - 구현 내용:
     - `TEST_LOG.md`/`CHANGELOG.md`: `[PHASE 05 STEP G]` 항목 추가(Debug/Release 빌드 성공, 실행 스모크 테스트 통과 — 24버텍스/36인덱스/1서브메시, 256바이트 CB/48바이트 payload 2개, D3D12 초기화·정상 종료, 오류/크래시 없음)
     - `PROMPT_LOG.md`: PHASE 05(PROMPT-0005) 섹션에 Step G 사용자 육안 확인 라인 추가, 미해결 사항 블록을 "Step G 완료… 육안 확인 완료로 PHASE 05 완료 처리", "카메라 이동 입력은 후속 작업(현재 정적 카메라)", "D3D12 debug layer validation 로그 미캡처"로 갱신
     - `TASKS.md`: PHASE 05를 `## 완료`로 이동, Step G `[x]` 체크, 다음 작업을 PHASE 05 후속(카메라 입력)/PHASE 06으로 갱신
     - `PROJECT_PLAN.md`: PHASE 05 상태를 완료(카메라 입력 연결 제외)로 갱신
   - 검증: **사용자 육안 확인 완료 — 씬 큐브 2개(`CenterCube`/`OffsetCube`) + 방향성 조명 정상 표시** ("큐브 두 개 나온다")
     - 미검증: D3D12 debug layer validation 메시지는 OutputDebugString으로 출력되어 로그에 남지 않음 → 로그상 오류 없음은 스모크 테스트 수준. 카메라 이동(WASD)·궤도 제어는 Step G 시점에는 UI 입력 연결이 없어 미구현이었음 → Step H에서 코드 연결 완료(입력 동작 육안 확인 대기).

8. **Step H — 카메라 입력 연결** (구현 완료, 입력 동작 육안 확인 대기)
   - 수정: `src/ChalkConfig.h`, `src/ChalkWindow.h`/`.cpp`, `src/ChalkGraphics.h`, `src/ChalkApp.h`/`.cpp`
   - Win32 메시지 기반 입력 상태 추적 후 렌더 루프 idle 콜백에서 카메라 구동
   - 구현 내용:
     - `ChalkConfig.h`: `kCameraMoveSpeed = 4.0f`(units/sec), `kCameraOrbitSensitivity = 0.006f`(radians/pixel), `kMaxFrameDeltaSeconds = 0.1f`(첫 프레임 dt 스파이크 clamp) 상수 추가
     - `ChalkWindow.h`/`.cpp`: `#include <array>`/`<cstddef>` 추가, `enum MouseButton : int { kMouseLeft=0, kMouseRight=1, kMouseMiddle=2, kMouseButtonCount=3 }`와 접근자 `IsKeyDown(int virtualKey)`/`IsMouseButtonDown(int button)`/`MousePosition()`/`ConsumeMouseDelta(int&,int&)`(consume+reset) 선언·정의. `OnMessage`에 `WM_KEYDOWN`/`WM_KEYUP`(상태 set, 0 반환), `WM_SYSKEYDOWN`/`WM_SYSKEYUP`(상태 set + `DefWindowProcW`), `WM_MOUSEMOVE`(누적 델타, `mouseTracked_`로 첫 이동 baseline 스킵), `WM_LBUTTON/RBUTTON/MBUTTONDOWN/UP`(`SetCapture`/`ReleaseCapture`), `WM_SETFOCUS`(`mouseTracked_ = false`), `WM_KILLFOCUS`(`ResetInputState()`) 처리 추가. private `ResetInputState()`, 멤버 `std::array<bool,256> keyDown_`, `std::array<bool,kMouseButtonCount> mouseDown_`, `POINT mousePosition_`, `int mouseDeltaX_/mouseDeltaY_`, `bool mouseTracked_`
     - `ChalkGraphics.h`: `ChalkCamera& Camera() noexcept` + const 오버로드 추가(camera_는 private 유지, 최소 변경)
     - `ChalkApp.h`/`.cpp`: 타이밍 멤버(`performanceFrequency_`/`lastFrameCounter_`/`hasLastFrameCounter_`)와 `ComputeDeltaSeconds()`/`UpdateCamera(float)` 추가. `Run()` 진입 시 `QueryPerformanceFrequency`/`QueryPerformanceCounter` 초기화, `ProcessMessages` idle 콜백에서 `ComputeDeltaSeconds()` → `UpdateCamera(dt)` → `graphics_->RenderFrame()` 순서로 호출. `#include "ChalkCamera.h"` 추가
     - 입력 매핑: W/S/A/D → `camera.MoveForward`/`camera.MoveRight`(이동량 = `kCameraMoveSpeed * dt`), 좌클릭 드래그 → `camera.Orbit(yaw, pitch)`(부호 `yaw = -dx*sens`, `pitch = -dy*sens`). `ComputeDeltaSeconds`는 QPC 차분으로 dt 계산, 음수 방지 + `kMaxFrameDeltaSeconds` clamp
   - 검증: Debug/Release 빌드 성공. 실행 스모크 테스트 통과(Release, 4초 실행 유지·D3D12 초기화 로그 정상·무크래시 종료)
   - 미검증: **입력 동작(WASD 이동/좌클릭 드래그 궤도) 사용자 육안 확인 대기** (합성 입력 주입 불가로 자동 검증 불가)

**빌드 및 실행 명령**:
- 빌드: `cmake --build build --config Debug` / `--config Release`
- 실행: `build\Debug\CHALKFORGE.exe` (또는 `build\Release\CHALKFORGE.exe`)
- 조작: W/S/A/D 이동, 마우스 좌클릭 드래그로 궤도 회전

**기술적 위험**:
- DirectXMath 정렬 요구 사항 (16-byte)
- Constant 버퍼 정렬 (256-byte 경계)
- 행렬 row-major vs column-major 불일치 (업로드 시 전치 누락)

**학습 목표**: DirectXMath 사용, 카메라 행렬, 메시 데이터 구조, 머티리얼 Constant 버퍼, 기본 씬 그래프

**검증 절차**:
1. 프로젝트 성공적 빌드 (Debug/Release) — 완료
2. 실행 파일 무크래시 실행 및 초기화 로그 확인 — 완료(스모크 테스트 통과)
3. 카메라 이동(WASD)·궤도 정상 — 구현 완료(빌드/실행 확인), 입력 동작 육안 확인 대기
4. 투영(원근) 행렬 정상 — 완료(육안 확인: 큐브가 원근감 있게 표시)
5. 머티리얼이 있는 메쉬 렌더링, 씬 객체 순회 정상 — 완료
6. 사용자 육안 확인 — 완료(씬 큐브 2개 + 방향성 조명 정상 표시)

**수락 기준**:
1. Debug/Release 빌드 성공 — 충족
2. 카메라 제어 가능 — 구현 완료(빌드/실행 확인), **입력 동작 육안 확인 대기**(미확인 항목으로 유지)
3. 메쉬가 머티리얼로 렌더링됨 — 충족
4. 씬 객체 관리 가능 — 충족
5. 사용자 동작 확인 완료 — 충족(육안 확인: 큐브 2개 + 조명 표시)

**미해결 이슈**:
- 좌표 체계 (row-major vs column-major) 컨벤션 — Step A에서 `XMMatrixTranspose` 방식으로 결정·기록 (DECISIONS.md 참조).
- 카메라 이동 입력(WASD/마우스 궤도) 코드 연결 완료(Step H, Debug/Release 빌드·실행 스모크 통과) — **입력 동작 사용자 육안 확인 대기**.
- D3D12 debug layer validation 메시지 미캡처(OutputDebugString) — 확인되지 않음.
