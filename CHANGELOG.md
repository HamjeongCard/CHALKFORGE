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
  - 미완료: 삼각형/깊이/리사이즈 사용자 육안 확인

### [Future Phases]
- To be populated as development progresses.