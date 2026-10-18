TASKS.md - CHALKFORGE 작업 추적
=====================================

## 진행 중
- PHASE 05 후속 Step H: 카메라 이동(WASD/마우스 궤도) 입력 연결 — **구현/빌드/실행 완료, 입력 동작 육안 확인 대기**
  - 수정: ChalkConfig.h(이동속도/궤도감도/최대 dt 상수), ChalkWindow.h/.cpp(키·마우스 입력 추적 + 접근자/ResetInputState), ChalkGraphics.h(`Camera()` 접근자), ChalkApp.h/.cpp(QPC 기반 `ComputeDeltaSeconds`, `UpdateCamera`, idle 콜백 연결)
  - 검증: Debug/Release 빌드 성공, Release 실행 스모크 테스트 통과(4초 실행 유지·D3D12 초기화 로그 정상·무크래시)
  - 미검증: 입력 동작(W/A/S/D 이동, 좌클릭 드래그 궤도) 사용자 육안 확인 대기

## 완료
- PHASE 05 카메라, 수학, 메시 및 씬 시스템 (Step A~G 완료):
  - [x] Step A: 문서 정합화 (PROJECT_PLAN/ROADMAP/TASKS/CODE_GUIDE/ARCHITECTURE/DECISIONS, docs/phases/PHASE_05.md 생성)
  - [x] Step B: ChalkMath 확장 (Matrix4x4 연산/투영/변환) + ChalkMath.cpp 신규 + CMakeLists.txt 소스 추가 — Debug/Release 빌드 성공
  - [x] Step C: ChalkCamera.h/.cpp — 위치/타겟/업, 뷰·프로젝션 행렬, WASD/마우스 궤도 + CMakeLists.txt 소스 추가 — Debug/Release 빌드 성공
  - [x] Step D: ChalkMesh.h/.cpp — 버텍스/인덱스 버퍼, 서브메쉬 관리(ChalkVertex/ChalkSubmesh) + CMakeLists.txt 소스 추가 — Debug/Release 빌드 성공
  - [x] Step E: ChalkMaterial.h/.cpp — Constant 버퍼(색상/조명 파라미터, 256-byte 정렬, 루트 CBV 바인딩) + CMakeLists.txt 소스 추가 — Debug/Release 빌드 성공 (텍스처 SRV는 이후 PHASE로 보류)
  - [x] Step F: ChalkScene.h/.cpp — 씬 객체 관리(Add/Remove/Get/순회) + 조명(Directional light CB) + ChalkGraphics 통합(카메라 VP·씬 순회·Transform/Material/Light CB·루트 서명 CBV 3슬롯·PSO 입력 레이아웃 ChalkVertex·셰이더 갱신) + CMakeLists.txt 소스 추가 — Debug/Release 빌드 성공, 실행 스모크 테스트 통과(ExitCode=0, 로그 오류 없음)
  - [x] Step G: 검증 + 문서화 — 문서 갱신(CHANGELOG/TEST_LOG/PROMPT_LOG/TASKS/PROJECT_PLAN/ROADMAP/docs/phases/PHASE_05.md). 사용자 육안 렌더 확인 완료(씬 큐브 2개 + 방향성 조명)
- PHASE 04 GPU 리소스 관리 및 렌더 파이프라인 안정화 (완료):
  - [x] Step A: 문서 정합화 (PROJECT_PLAN/ROADMAP/TASKS/CODE_GUIDE/ARCHITECTURE/DECISIONS, docs/phases/PHASE_04.md 생성)
  - [x] Step B: ChalkFence.h/.cpp — 펜스/이벤트/per-frame 값 캡슐화 (Signal/WaitForValue/WaitForGpu/CompletedValue) — Debug/Release 빌드 성공
  - [x] Step C: ChalkBarrier.h/.cpp — Transition 도우미 (인라인 barrier 대체) — Debug/Release 빌드 성공
  - [x] Step D: ChalkResource.h/.cpp — ID3D12Resource 래퍼 + 현재 state + 이름 태그 + 수명 등록/해제 로깅 (깊이 버퍼 생성 도우미 포함) — Debug/Release 빌드 성공
  - [x] Step E: 트리플 버퍼링 (kSwapChainBufferCount=3) + 백 버퍼별 깊이 버퍼/DSV/barrier + PSO DepthEnable — Debug/Release 빌드 성공
  - [x] Step F: 렌더 루프/동기화 고도화 (모든 D3D12 생성 콜 오류 확인, k프레임 in-flight, 해제 순서 검증) — Debug/Release 빌드 성공
  - [x] Step G: 검증 + 문서화 (Debug/Release 빌드 성공, 실행 로그 오류 없음, 사용자 동작 확인 완료)
- PHASE 00 설정:
  - [x] 프로젝트 디렉토리 및 git 저장소 조사
  - [x] 컴파일러/SDK/도구 환경 확인
  - [x] 요구사항 및 기술적 위험 분석
  - [x] 프레임워크 아키텍처 초안 작성
  - [x] 단계별 개발 로드맵 생성 (PHASE 00-14)
  - [x] 문서화 시스템 설정
  - [x] PROMPT_LOG.md에 첫 프롬프트 기록
  - [x] 교차 문서 일관성 검토
  - [x] Phase 00 완료 보고서 생성
- PHASE 01 설정 (부분 완료):
  - [x] CMakeLists.txt 생성 (Visual Studio 2022 generator)
  - [x] 기본 Windows 애플리케이션 진입점 구현 (src/ChalkApp.cpp)
  - [ ] 로깅 시스템 (ChalkLogger) → PHASE 03에서 구현
  - [ ] 오류 처리 (ChalkError) → PHASE 03에서 구현
  - [ ] 구성 시스템 (ChalkConfig) → PHASE 03에서 구현
  - [ ] 수학 유틸리티 (ChalkMath) → PHASE 03에서 최소 구현, PHASE 05 확장
- PHASE 02 설정 (완료):
  - [x] Win32 창 생성 구현 (ChalkApp.cpp)
  - [x] /subsystem:windows CMake 플래그 적용
- PHASE 03 Direct3D 12 초기화 및 기본 렌더링 (완료):
  - [x] Step A: 문서 정합화 (ROADMAP/TASKS/CODE_GUIDE/DECISIONS/PROJECT_PLAN)
  - [x] Step B: build/ Git 추적 해제 및 .gitignore에 build/ 추가
  - [x] Step C: PHASE 01 기반 파일 구현 (ChalkLogger, ChalkError, ChalkConfig, ChalkMath, ChalkApp.h)
  - [x] Step D: CMake 확장 (d3d12/dxgi/d3dcompiler 링크, /utf-8, Debug 디버그 레이어)
  - [x] Step E: ChalkApp.cpp 리팩터링 (ChalkApplication/ChalkWindow, UNICODE + Per-Monitor V2 DPI)
  - [x] Step F: ChalkGraphics D3D12 초기화 → 백 버퍼 클리어 → 파이프라인 → 삼각형 → GPU 동기화 → 리사이즈
  - [x] Step G: 단계 문서(docs/phases/PHASE_03.md) 및 CHANGELOG/PROMPT_LOG/TEST_LOG 갱신
  - [x] 삼각형 화면 출력 및 창 리사이즈 동작 사용자 육안 확인 완료

## 다음 작업
- PHASE 05 후속(선택): 카메라 이동(WASD/마우스 궤도) 입력 연결 — PHASE 05 범위 중 미구현 항목
- PHASE 06: 3D 칠판 및 머티리얼 시스템 (ROADMAP.md 참조)
- 미확인 항목: D3D12 debug layer validation 메시지 미캡처(로그에는 앱 메시지만 기록됨)