DECISIONS.md - CHALKFORGE 기술 결정사항
==============================================

## 빌드 시스템
### 결정: CMake 기반 빌드 with Visual Studio 2022 generator
**날짜**: 2026-10-12
**선택된 대안**: CMake vs. Visual Studio solution file

**선택 이유**:
1. **크로스 플랫폼 가능성**: CMake는 미래의 Linux/macOS 포트 가능하게 함
2. **현대 C++ 프로젝트 표준**: C++ 프로젝트의 업계 표준
3. **타사 의존성 관리 간소화**: FetchContent, vcpkg 통합
4. **적절한 VS 솔루션 생성**: 필요 시 .sln 생성 가능하지만 CMake가 기본
5. **빌드 플래그 제어 개선**: 컴파일러 플래그, 정의, include 경로

**거부된 대안**: 직접 Visual Studio 솔루션 (.sln)
- Windows 전용 즉시 개발에 더 간단함
- 미래 변화에 덜 유연함
- 수동 프로젝트 파일 관리 필요

**결정 후속 조치**:
- CMake 3.15+ 필요 (VS 2022와 가능성 있게 포함됨)
- `cmake -G "Visual Studio 17 2022"` 제너레이터 사용
- MSVC 전용 플래그는 CMake 또는 타겟 프로퍼티를 통해 처리

---

## Direct3D 12 기능 수준
### 결정: D3D_FEATURE_LEVEL_11_0을 최소값으로, D3D_FEATURE_LEVEL_12_1 타겟
**날짜**: 2026-10-12

**선택된 대안**: 기능 수준 선택 전략

**선택된 내용**:
- 주요 타겟: D3D_FEATURE_LEVEL_12_0
- 최소 지원: D3D_FEATURE_LEVEL_11_0
- 폴백 체인: 12_0 → 11_1 → 11_0

**선택 이유**:
1. **하드웨어 현실**: 대부분의 타겟 하드웨어에서 최소 Feature Level 11_1 지원
2. **D3D12 요구 사항**: 핵심 D3D12 기능 (명령 큐, 기술 힙, 루트 서명) 사용 가능 at 11_0
3. **고급 기능**: 12_0은 명령 목록, 그래픽 파이프라인_ENABLE; 12_1은 샘플러 피드백, 메쉬 셰이더 (향후)
4. **광범위한 호환성**: 최소 11_0은 더 넓은 하드웨어 지원

**거부된 대안**: D3D_FEATURE_LEVEL_12_1 only
- 오래된 하드웨어 배제
- CHALKFORGE 핵심 기능에 불필요

**결정 후속 조치**:
- 디바이스 생성 시 기능 수준 선택 (D3D12CreateDevice)
- 실제 지원 수준 보고를 위한 적응형 기능 수준 로깅
- 기능 수준별 리소스 생성 (12_1 기능은 선택적)

---

## 칠크 렌더링 전략
### 결정: vertex shader 폭 제어를 이용한 기하학 기반 튜브 렌더링
**날짜**: 2026-10-12 (PHASE 00 계획)

**선택된 대안**: 텍스처 기반 vs. 기하학 기반 스트로크 렌더링

**선택된 내용**: 스트로크당 기하학 기반 렌더링 (튜브/리본 메쉬)
- 버텍스 버퍼: 위치 + 법선 + 색상당 버텍스
- 폭 제어를 통한 vertex 간격 또는 셰이더 스케일링
- 스트로크당 기하학 생성 (스트로크 완료 시 또는 동적)
- Multisampling을 통한 라인 앤티 앨리어싱 지원
- 보드와의 적절한 3D 깊이 테스트/상호작용 지원

**거부된 대안**: 텍스처 기반 렌더링 (전체 화면 퀼드 또는 텍스처 어트라)
- 더 간단한 초기 구현
- 3D 폭/깊이 지각에 덜 유연함
- 적절한 깊이 힌트 유지 어려움
- 텍사스 크기에 독립적인 폭 제어 제한적

**결정 후속 조치**:
- CPU 상에서 스트로크 기하학 생성 (점 목록 기반 버텍스 추가)
- 버텍스 셰이더: 위치 변환, 픽셀로 색상 전달
- 픽셀 셰이더: 간단한 flat 색상 또는 단순한 조명
- GPU 버퍼 관리 (CPU 측 스트로크 데이터 → GPU 버텍스 버퍼)
- 스트로크당 폭 파라미터, 라인 폭 라스터라이저 또는 버텍스 간격으로 렌더링

**고려된 대안**: 하이브리드 접근 (기하학 + 프로그램적 텍스처 좌표를 통한 색상 칠하기)
- 미래 향상 (PHASE 13+)로 보류
- 초기 구현에 불필요한 복잡성 추가

---

## 입력 시스템
### 결정: WM_MOUSEMOVE 메시지 기반 입력 (주기적 샘플링)
**날짜**: 2026-10-12 (PHASE 02 계획)

**선택된 대안**: WM_MOUSEMOVE vs. RAWINPUT vs. GetCursorPos

**선택된 내용**: 초기 단계에 충분한 단순성
- RAWINPUT 오버헤드 또는 디바이스 열거 복잡성 불필요
- 사용자 드래그 속도로 관리 가능한 샘플 레이트
- 표준 Win32 접근 방식으로 호환성 보장

**거부된 대안**: RAWINPUT (직접 입력)
- 더 정확한 마우스 추적
- 디바이스 등록, 획득/해제 필요
- 초기 CHALKFORGE 범위에 불필요한 복잡성
- 게임 애플리케이션에 더 적합, 유틸리티 아님

**결정 후속 조치**:
- 마우스 상태 추적 (위치, 버튼 상태, 델타)
- 입력 샘플링 레이트: 60Hz 또는 프레임당 임계값
- 스트로크 생성을 위한 최소 점 거리 임계값 (예: 2.0 픽셀)
- 단일 클릭 감지 (mousedown + mouseup significativa 움직임 없음)
- 좌표 변환: 창 → 클라이언트 → 스크린 → NDC → 월드

---

## 창 생성
### 결정: 표준 Win32 WNDCLASS with CS_DBLCLKS 및 CS_HREDRAW/VREDRAW
**날짜**: 2026-10-12 (PHASE 02 계획)

**선택된 대안**: Win32 창 서브클래스 vs. 프레임워크 래퍼 vs. Dear ImGui 네이티브 창

**선택된 내용**: 손수 만든 Win32 창 절차
- 창 메시에 대한 전체 제어
- 외부 UI 프레임워크 의존성 없음
- 초기 단계에 가벼움 (ImGui 오버헤드 없음)
- D3D12 애플리케이션에 표준 접근 방식

**거부된 대안**: Dear ImGui 창 등
- ImGui 통합 PHASE 14로 연기
- 초기 단계에 불필요한 종속성 추가
- 창 절차 복잡성은 CHALKFORGE 내에서 관리

**결정 후속 조치**:
- 적절한 플래그가 있는 WNDCLASS
- WS_OVERLAPPEDWINDOW 스타일로 CreateWindowEx
- SetProcessDPIAwareness(PDT_AWARE) 고해상도 디스플레이용
- 창 서브클래스 (WndProc)가 D3D12별 메시 처리
- 클라이언트 영역 크기 추적을 스왑 체인 및 렌더링용

---

## 수학 라이브러리
### 결정: DirectXMath (XMVECTOR/XMMATRIX) with 16-byte alignment
**날짜**: 2026-10-12 (PHASE 05 계획)

**선택된 대안**: DirectXMath vs. glm vs. 사용자 정의 수학 라이브러리

**선택된 내용**: DirectXMath (Microsoft의 SIMD 수학 라이브러리)
- Windows SDK와 함께 제공, 외부 의존성 없음
- MSVC SIMD 내포 최적화에 최적화
- HLSL과 직접 통합 (동일한 유형, 동일한 SIMD 레이아웃)
- CHALKFORGE 기능 세트에 충분함

**거부된 대안**: glm (OpenGL Mathematics)
- 인기 있음, 크로스 플랫폼
- 다른 SIMD 레이아웃 관행 (column-major vs DirectX row-major)
- 추가 의존성 관리
- 좌표 시스템 변환 오버헤드

**고려된 대안**: 사용자 정의 수학 라이브러리
- 유형 및 컨벤션에 대한 전체 제어
- 상당한 개발 노력
- SIMD 최적화 필수 anyway
- DirectXMath가 자유롭게 사용할 수 있을 때 justify되지 않음

**결정 후속 조치**:
- <DirectXMath.h> 포함 (필요한 경우 compiler flags /Zc:__cplusplus-)
- 16-byte 정렬과 함께 XMVECTOR/XMMATRIX 사용 (__declspec(align(16)))
- matrix transpose 컨벤션 인지 (DirectX는 row-major; OpenGL은 column-major)
- HLSL Constant buffer 정렬이 C++ struct 레이아웃과 일치하도록 함
- CPU 측에서 colors/vectors를 XMFLOAT types (float4)에 저장, SIMD 연산을 위해 XMVECTOR로 변환

---

## 데이터 직렬화 형식
### 결정: 버전 헤더가 있는 사용자 정의 바이너리 형식
**날짜**: 2026-10-12 (PHASE 12 계획)

**선택된 대안**: 바이너리 vs. JSON vs. XML

**선택된 내용**: 사용자 정의 바이너리 형식
- 최소 파일 크기 (whitespace 오버헤드 없음)
- 빠른 파싱/쓰기 (JSON 파싱 오버헤드 없음)
- endianness 제어권 (little-endian, native)
- 형식 진화를 위한 버전 헤더
- 구조화된 스트로크/보드/카메라 데이터 레이아웃이 단순함

**거부된 대안**: JSON (RapidJSON 또는 nlohmann)
- 사람에 의해 읽기 쉬움 (디버깅 용이함)
- verbose 파일 크기 (whitespace, key 문자열)
- 로드 시 파싱 오버헤드
- 외부 의존성 또는 번들된 파서 필요
- 구조화된 스트로크 데이터에 오버kill

**고려된 대안**: protobuf
- 뛰어난 버전링 및 진화
- 바이너리 크기 효율적
- CHALKFORGE 범위를 초과함
- 네트워크/멀티앱 데이터 교환에 더 적합

**결정 후속 조치**:
- 파일 헤더: 4바이트 매직 + 4바이트 형식 버전 + 4바이트 데이터 크기
- 스트로크 데이터: 점 수 + (위치 + 색상 + 폭)당
- 보드 데이터: 치수, 위치, 머티리얼 속성
- 카메라 데이터: 위치, 타겟, up, projection 파라미터
- endianness: native (x86/x64 Windows에서 little-endian)
- 파일 확장자: .chb (ChalkBoard)

**유지할 대안**: JSON 디버깅이 필요할 경우, 보조 도구로 빠른 serializer 추가, 주요_format 아님.

---

## Undo/Redo 명령 패턴
### 결정: 타입별 enum + 변형 페이로드가 있는 명령 패턴
**날짜**: 2026-10-12 (PHASE 10 계획)

**선택된 대안**: 명령 패턴 vs. Memento vs. State 패턴

**선택된 내용**: 타입이 있는 enum + 데이터 페이로드가 있는 명령 패턴
- 명시적 명령 유형 (StrokeAdd, StrokeRemove, ColorChange, WidthChange, Erase, ClearBoard)
- 명령당 minimal bytes의 union/struct
- 스택 기반 Undo/Redo (std::vector<Command>)
- 구성 가능한 최대 이력 깊이 (예: 200명령)
- 체크포인트 시스템 (주기적 전체 상태 스냅샷, 예: 50명령마다)

**거부된 대안**: Memento 패턴 (전체 상태 스냅샷)
- 구현 단순함
- 큰 메모리 footprint (명령당 전체 보드 상태 복사)
- 세부 단위 Undo 어려움 (한번에 하나씩 스트로크 undo 어려움)
- 많은 연산에 대해 스케일링 안됨

**고려된 대안**: State 패턴
- 시스템-wide 상태 관리에 좋음
- CHALKFORGE에 오버킬
- 개별 명령 역전 직렬화 어려움

**결정 후속 조치**:
- CommandId enum: STROKE_ADD, STROKE_REMOVE, COLOR_CHANGE, WIDTH_CHANGE, ERASE, CLEAR_BOARD
- type-specific 필드가 있는 CommandPayload struct (점 데이터, 색상, 폭, 지우개 반지름)
- UndoRedoManager 클래스 with undo_stack_, redo_stack_, max_depth_, checkpoint_interval_
- 명령당 serialize/deserialize 메서드 for file persistence

---

## 좌표 컨벤션
### 결정: Y-up인 우측 좌표계
**날짜**: 2026-10-12 (초기 계획)

**선택된 대안**: 우측 Y-up vs. 좌측 Z-up vs. 사용자 정의

**선택된 내용**: Y-up인 우측 좌표계 (DirectXMath 기본)
- DirectXMath 기본값은 우측-handed, Y-up
- HLSL 효과 및 샘플 코드 대부분이 이 컨벤션 사용
- 카메라: -Z축을 look at, Y는 up
- 보드 평면: 수평 XZ 평면 또는 수직 XY 평면 (보드 방향에 따라 결정)

**거부된 대안**: 좌측 Z-up (일부 버전의 기본 Direct3D)
- DirectXMath 우측-handed 컨벤션과 혼동됨
- 상수 변환 매트릭스 필요
- 샘플 코드 불일치

**결정 후속 조치**:
- 모든 행렬: DirectXMath XMMATRIX 연산
- 투영: PerspectiveLH 또는 PerspectiveFovLH (left-handed projection but right-handed world)
- 뷰: LookLH 또는 Inverse(LookToLH)
- 전체적으로 일관된 사용; 좌표계 혼합 금지

---

## PHASE 03 — D3D12 구조 및 기반 결정
### 결정: D3D12 자원을 단일 `ChalkGraphics` 클래스로 캡슐화
**날짜**: 2026-10-18

**선택된 대안**: 단일 통합 클래스 vs. 자원별 개별 클래스 (ChalkDevice, ChalkCmdQueue 등 9개)

**선택된 내용**: PHASE 03의 디바이스, 명령 큐, 명령 할당자/목록, 스왑 체인, RTV 힙, 루트 서명, PSO를 `ChalkGraphics` 한 클래스에 내부 캡슐화한다.

**선택 이유**:
- 초기 단계에서 자원 간 결합이 강해 자원별 파일 분리는 과도한 추상화
- 문서(ROADMAP/CODE_GUIDE)와 단일 클래스 명명 일치
- 이후 PHASE 04에서 필요 시 자원별 파일로 분리 가능

**거부된 대안**: 자원별 9개 클래스 즉시 분리 — 파일/문서 수 증가, 조기 설계 위험

### 결정: 셰이더 런타임 컴파일 (d3dcompiler.lib)
**날짜**: 2026-10-18

**선택된 내용**: `shaders/*.hlsl`을 실행 시 `D3DCompile()`로 컴파일한다.

**선택 이유**: 별도 빌드 스텝 없이 셰이더 수정 즉시 반영, 학습에 유리

**거부된 대안**: 사전 컴파일(.cso) + 빌드 스텝 — 초기 단계에 불필요한 빌드 복잡성

### 결정: UNICODE + Per-Monitor V2 DPI 적용 (기존 ANSI 코드 전환)
**날짜**: 2026-10-18

**선택된 내용**: PHASE 03에서 `CreateWindowExW`, 와이드 문자열, `SetProcessDpiAwarenessContext(PER_MONITOR_AWARE_V2)`를 적용한다.

**선택 이유**: 고해상도 디스플레이 대응, 향후 UI 단계 대비. 기존 PHASE 02의 ANSI 코드를 조기에 전환

**영향**: `src/ChalkApp.cpp` 리팩터링 필요 (PHASE 03 범위)

### 결정: D3D12 디버그 레이어 전처리기 정의 및 /utf-8 컴파일 옵션
**날짜**: 2026-10-18

**선택된 내용**: Debug 구성에 `CHALK_DEBUG_LAYER` 전처리기를 generator expression(`$<$<CONFIG:Debug>:CHALK_DEBUG_LAYER>`)으로 정의하고, MSVC 컴파일러에 `/utf-8`을 적용한다. Release에는 디버그 레이어 정의를 넣지 않는다. `CMakeLists.txt`에서 `d3d12`/`dxgi`/`d3dcompiler`를 링크한다.

**선택 이유**:
- `_DEBUG` 대신 명시적 매크로로 릴리스/디버그 제어를 컴파일러 독립적으로 분리
- `/utf-8`로 소스/실행 문자 집합 모호성 제거
- Debug 전용 정의로 릴리스 성능·바이너리 영향 없음

**거부된 대안**: `_DEBUG` 매크로 의존 — 컴파일러·표준 라이브러리 구현에 따라 정의 시점이 달라 명시적 제어 불가

**영향**: 디버그 레이어 활성화 코드는 Step F의 `ChalkGraphics`에서 `CHALK_DEBUG_LAYER`로 게이트한다.

### 정책: CMake 생성물(build/) Git 추적 해제
**날짜**: 2026-10-18

**내용**: `build/`는 생성 산출물이므로 추적에서 제외하고 `.gitignore`에 추가한다. (PHASE 03 착수 시 수행)

---

## 주요 결정 사항 요약

| 카테고리 | 결정 | 타당성 |
|----------|------|--------|
| 빌드 시스템 | CMake + VS 2022 제너레이터 | 유연성, 현대 표준, 미래지향 |
| 기능 수준 | 12_0 최소, 11_0 폴백 | 광범위 호환성, 핵심 D3D12 기능 |
| D3D12 구조 | ChalkGraphics 단일 클래스 | 통합 관리, 과도한 추상화 회피 |
| 셰이더 | 런타임 컴파일 (d3dcompiler) | 즉시 반영, 빌드 단순 |
| 문자/DPI | UNICODE + Per-Monitor V2 | 고해상도 대응, 선제 전환 |
| 렌더링 | 기하학 기반 튜브 | 3D 폭 지각, 적절한 깊이 |
| 입력 | WM_MOUSEMOVE 샘플링 | 단순성, 그리기에 충분 |
| 창 | 손수 만든 Win32 | 완전한 제어, 경량 |
| 수학 | DirectXMath | 추가 종속성 없음, HLSL 통합 |
| 형식 | 사용자 정의 바이너리 | 빠름, 작음, 버전화됨 |
| Undo/Redo | 명령 패턴 | 세부 단위, 메모리 효율적 |
| 좌표 | 우측 Y-up | DirectXMath 기본값, HLSL 일관성 |

**다음 결정 지점**: PHASE 04 - 자원별 파일 분리 여부; PHASE 08 - 렌더링 전략 선택 검증.