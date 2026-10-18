TASKS.md - CHALKFORGE 작업 추적
=====================================

## 진행 중
- PHASE 04 GPU 리소스 관리 및 렌더 파이프라인 안정화:
  - [x] Step A: 문서 정합화 (PROJECT_PLAN/ROADMAP/TASKS/CODE_GUIDE/ARCHITECTURE/DECISIONS, docs/phases/PHASE_04.md 생성)
  - [x] Step B: ChalkFence.h/.cpp — 펜스/이벤트/per-frame 값 캡슐화 (Signal/WaitForValue/WaitForGpu/CompletedValue) — Debug/Release 빌드 성공
  - [x] Step C: ChalkBarrier.h/.cpp — Transition 도우미 (인라인 barrier 대체) — Debug/Release 빌드 성공
  - [x] Step D: ChalkResource.h/.cpp — ID3D12Resource 래퍼 + 현재 state + 이름 태그 + 수명 등록/해제 로깅 (깊이 버퍼 생성 도우미 포함) — Debug/Release 빌드 성공
  - [x] Step E: 트리플 버퍼링 (kSwapChainBufferCount=3) + 백 버퍼별 깊이 버퍼/DSV/barrier + PSO DepthEnable — Debug/Release 빌드 성공
  - [x] Step F: 렌더 루프/동기화 고도화 (모든 D3D12 생성 콜 오류 확인, k프레임 in-flight, 해제 순서 검증) — Debug/Release 빌드 성공
  - [ ] Step G: 검증 + 문서화 (Debug/Release 빌드 성공, 실행 로그 확인 완료 / 사용자 육안 확인 대기)

## 완료
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
- PHASE 04 Step A(문서 정합화) → Step B~G 구현 순서로 진행
  - 신규 파일: ChalkResource, ChalkFence, ChalkBarrier (Step B~D)
  - 트리플 버퍼링 + 깊이 버퍼 통합 (Step E)
  - 동기화/오류 확인 고도화 (Step F), 검증·문서화 (Step G)
- PHASE 05 (카메라, 수학, 메시 및 씬 시스템)는 PHASE 04 완료 후 착수