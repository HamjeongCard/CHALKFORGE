PHASE 04 — GPU 리소스 관리 및 렌더 파이프라인 안정화

**목표**: robust GPU 리소스 관리 및 렌더 파이프라인 패턴을 확립한다. 명령 할당자/목록 기록 패턴, 리소스 바리어, 깊이 버퍼, 트리플 버퍼링, 펜싱 기반 동기화를 정식 헬퍼 클래스로 분리·구현하고, 모든 D3D12 생성 콜의 오류를 확인한다.

**구현 완료 상태**: 코드 구현(Step A~F) 완료, Debug/Release 빌드 성공 및 실행 로그 검증 완료. Step G(문서화 및 사용자 육안 확인) 진행 중 — 육안 확인(삼각형 + 깊이 + 리사이즈)은 사용자가 확인하기 전까지 완료로 표기하지 않는다.

**내부 검증 단계** (각 항목을 개별 검증):
1. 펜싱 — `ChalkFence`로 프레임 간 자원 재사용 안전 (기존 fence 로직 이관)
2. 리소스 바리어 — `ChalkBarrier` 도우미로 상태 전이 정상 (PRESENT↔RT, DEPTH_WRITE)
3. 리소스 수명 — `ChalkResource`로 CPU 측 핸들 추적 및 해제 시 누수 없음
4. 트리플 버퍼링 — 3개 프레임 자원으로 안정적 멀티 프레임 렌더링
5. 깊이 버퍼 — 백 버퍼별 깊이 리소스/DSV 부착, 깊이 테스트 활성화
6. 리사이즈 — 깊이 버퍼 포함 전 자원 재생성 정상
7. 오류 확인 — 모든 D3D12 생성 콜 오류 처리(FAIL)

**핵심 설계 결정** (DECISIONS.md 참조):
- 트리플 버퍼링: `kSwapChainBufferCount = 3`, `kFrameCount = 3` (기존 `MoveToNextFrame` 로직이 N프레임 일반화되어 있어 안전)
- 깊이 버퍼: 백 버퍼별 깊이 리소스 + DSV 힙 + barrier 관리, 리사이즈 시 전부 재생성 (표준 D3D12 멀티버퍼 패턴)
- 자원 분리: `ChalkResource`/`ChalkFence`/`ChalkBarrier` 3개 클래스를 실제 구현하고 `ChalkGraphics`가 멤버로 소유·조립 (과도한 추상화 회피, PHASE 05+ 확장 여지 확보)

**작업 단계 및 변경 예정 파일**:

1. **Step A — 문서 정합화** (완료)
   - 수정: `PROJECT_PLAN.md`, `ROADMAP.md`, `TASKS.md`, `CODE_GUIDE.md`, `ARCHITECTURE.md`, `DECISIONS.md`
   - 신규: `docs/phases/PHASE_04.md` (본 문서)

2. **Step B — `ChalkFence`** (완료)
   - 신규: `src/ChalkFence.h`/`.cpp` (작성·Debug/Release 빌드 성공)
   - 펜스/이벤트/per-frame 값 캡슐화. `Signal`→`uint64_t` 반환, `WaitForValue`, `WaitForFrame`, `WaitForGpu`, `CompletedValue`, `FrameValue`/`SetFrameValue`
   - `ChalkGraphics`의 기존 fence 로직 이관(`fence_`/`fenceEvent_`/`frameFenceValues_` → `ChalkFence fence_`), `CMakeLists.txt`에 `src/ChalkFence.cpp` 추가
   - 검증: Debug/Release 빌드 성공, stale 참조 없음 확인

3. **Step C — `ChalkBarrier`** (완료)
   - 신규: `src/ChalkBarrier.h`/`.cpp`, `CMakeLists.txt`에 `src/ChalkBarrier.cpp` 추가
   - `Transition(list, resource, before, after)` 도우미 구현. `PopulateCommandList`의 인라인 barrier 2곳(PRESENT↔RT) 대체
   - 검증: Debug/Release 빌드 성공, 인라인 barrier stale 참조 없음

4. **Step D — `ChalkResource`** (완료)
   - 신규: `src/ChalkResource.h`/`.cpp`, `CMakeLists.txt`에 `src/ChalkResource.cpp` 추가
   - `ID3D12Resource` 래퍼 + 현재 state(`State`/`SetState`) + 이름/디버그 태그 + 수명 등록·해제 로깅(CPU 핸들 추적), `CreateDepthBuffer(ID3D12Device*, ID3D12DescriptorHeap*, UINT dsvIndex, UINT width, UINT height, DXGI_FORMAT, const wchar_t* name)` 도우미
   - 검증: Debug/Release 빌드 성공

5. **Step E — 트리플 버퍼링 + 깊이 버퍼 통합** (완료)
   - 수정: `src/ChalkConfig.h`(`kSwapChainBufferCount=3`), `src/ChalkGraphics.h`/`.cpp`
   - DSV 힙 + 백 버퍼별 깊이 리소스(`depthStencils_[kFrameCount]`) + PSO `DepthEnable=TRUE`/`DepthWriteMask=ALL`/`DepthFunc=LESS`/DSVFormat=D32
   - `OMSetRenderTargets`에 DSV + `ClearDepthStencilView`(초기 상태 DEPTH_WRITE로 생성해 프레임별 barrier 불필요) + `OnResize` 및 `ReleaseResources`에서 깊이 리소스/DSV 힙 재생성·해제
   - 구현 시 시그니처 변경: `CreateDepthStencilBuffer(int width, int height)` — `Initialize`에서 `width_`/`height_` 대입이 Create* 호출보다 늦어 무인자 시 순서 버그가 발생하므로 파라미터로 전달
   - 검증: Debug/Release 빌드 성공

6. **Step F — 렌더 루프/동기화 고도화** (완료)
   - 수정: `src/ChalkGraphics.cpp`
   - `OnResize`에서 `CreateRenderTargets()`/`CreateDepthStencilBuffer(width, height)` 결과를 `FAIL` 없이 검사하고 실패 시 `Error` 로깅 후 반환하도록 하드닝
   - D3D12 HRESULT 콜 `FAIL()` 래핑 전수 확인, N프레임 in-flight(`frameValues` 기반) 및 해제 순서(역의존) 검토 완료
   - 검증: Debug/Release 빌드 성공

7. **Step G — 검증 + 문서화** (진행 중)
   - 수정: `CHANGELOG.md`, `TEST_LOG.md`, `PROMPT_LOG.md`, `TASKS.md`, 본 문서
   - Debug/Release 빌드 성공, 실행 로그(오류 없음) 확인 완료
   - 미완료: 사용자 육안 확인(삼각형 + 깊이 동작 + 리사이즈)

**CMakeLists.txt 변경**: `ChalkFence.cpp`, `ChalkBarrier.cpp`, `ChalkResource.cpp` 추가 완료.

**빌드 및 실행 명령**:
- 빌드: `cmake --build build --config Debug` / `--config Release`
- 실행: `build\Debug\CHALKFORGE.exe`

**기술적 위험**:
- 부적절한 펜싱으로 인한 리소스 누수
- 명령 큐 동기화에서 데드락
- 리사이즈 경로에서 깊이 버퍼 재생성 누락 시 뷰포트/DSV 불일치

**학습 목표**: 명령 할당자 재활용, 리소스 바리어 의미, 펜싱 의미, 트리플 버퍼링, GPU-CPU 동기화, 리소스 수명 추적

**검증 절차**:
1. 프로젝트 성공적 빌드 (Debug/Release)
2. 실행 파일 무크래시 실행 및 초기화 로그 확인
3. 리소스 누수 없이 여러 프레임 렌더링 (펜싱 동기화 작동)
4. 창 리사이즈 시 깊이 버퍼 포함 자원 재생성 정상
5. 사용자 육안 확인 (삼각형 + 깊이 동작 + 리사이즈)

**수락 기준**:
1. Debug/Release 빌드 성공
2. 안정적인 멀티 프레임 렌더링
3. 리소스 할당 오류 없음
4. 적절한 펜싱 신호
5. 사용자 육안 확인 완료 (미확인 시 완료 표기 금지)

**미해결 이슈**:
- 사용자 육안 확인(삼각형 + 깊이 동작 + 리사이즈) 미완료 — 확인 전까지 PHASE 04 완료로 표기하지 않는다.
