TEST_LOG.md - CHALKFORGE 테스트 로그
===================================

아직 실행된 테스트가 없습니다. 이 로그는 모든 빌드 결과, 테스트 실행, 오류 및 해결 기록을 담습니다.

형식:
### [PHASE 00] - 2026-10-12
- 환경 검증: MSVC, Windows SDK, Direct3D 12 헤더/라이브러리 확인
- 문서 일관성: 모든 PHASE 00 문서 생성 및 검토
- 빌드 결과: N/A (아직 구현 코드 없음)
- 테스트 결과: N/A

### [PHASE 01] - YYYY-MM-DD
- 단계가 진행됨에 따라 채워질 예정

### [PHASE 02] - 2026-10-12
- 빌드 결과: Debug/Release 빌드 성공
- 테스트 결과: "CHALKFORGE" 창 표시 및 정상 종료 확인

### [PHASE 03] - 2026-10-19
- 빌드 결과: Debug/Release 빌드 성공 (경고 없음)
  - `cmake --build build --config Debug` → build/Debug/CHALKFORGE.exe 생성, 셰이더 복사 확인
  - `cmake --build build --config Release` → build/Release/CHALKFORGE.exe 생성
- 파일 확인: build/Debug, build/Release 실행 파일 및 shaders/TriangleVS.hlsl, TrianglePS.hlsl 존재 확인
- 실행 결과: build/Debug/CHALKFORGE.exe 3초 실행 무크래시
- 로그 결과: chalkforge.log 3줄 확인, 오류 없음
  - [INFO] CHALKFORGE initializing
  - [INFO] D3D12 debug layer enabled.
  - [INFO] D3D12 graphics initialized (1280x720).
- 육안 검증 (사용자 확인 완료): 삼각형 화면 출력 정상 표시, 창 리사이즈 시 백 버퍼/뷰포트 정상 갱신 → PHASE 03 완료

### [PHASE 04] - 2026-10-19
- 빌드 결과: Debug/Release 빌드 성공 (경고 없음)
  - `cmake --build build --config Debug` → build/Debug/CHALKFORGE.exe 생성
  - `cmake --build build --config Release` → build/Release/CHALKFORGE.exe 생성
- 실행 결과: build/Debug/CHALKFORGE.exe 약 4초 실행 후 정상 종료(무크래시)
- 로그 결과(`build/Debug/chalkforge.log`, 오류 없음):
  - [INFO] CHALKFORGE initializing
  - [INFO] D3D12 debug layer enabled.
  - [INFO] Depth buffer created (1280x720). ×3 (백 버퍼별 깊이 버퍼)
  - [INFO] D3D12 graphics initialized (1280x720).
  - [INFO] ChalkResource released. ×3
  - [INFO] CHALKFORGE shutting down
- 자동 검증 완료: 빌드, 실행, 초기화/해제 로그
- 미완료(사용자 육안 확인 대기): 삼각형 화면 출력 유지, 깊이 테스트 동작, 창 리사이즈 시 깊이 버퍼 포함 재생성

### 오류 해결 기록
- C4244 (wide→narrow 변환 경고): ChalkGraphics.cpp 익명 네임스페이스에 로컬 WideToUtf8(wstring→UTF-8) 헬퍼 추가, 셰이더 경로 변환에 적용 → 경고 제거
- 링커 오류: d3d12/dxgi/d3dcompiler 라이브러리 링크 누락 → CMakeLists.txt target_link_libraries에 추가로 해결
- 한글 경로 콘솔 출력 깨짐: MSVC /utf-8 컴파일 옵션 추가로 해결