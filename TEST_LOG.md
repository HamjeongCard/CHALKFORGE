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
- 사용자 동작 확인 완료(사용자 보고 "작동 정상"): 삼각형 화면 출력 유지, 깊이 버퍼, 창 리사이즈 시 깊이 버퍼 포함 재생성 → PHASE 04 완료

### [PHASE 05 STEP F] - 2026-10-19
- 빌드 결과: Debug/Release 빌드 성공 (경고 없음)
  - `cmake --build build --config Debug` → build/Debug/CHALKFORGE.exe 생성 (CMake 재구성 + ChalkScene.cpp 컴파일 + 링크)
  - `cmake --build build --config Release` → build/Release/CHALKFORGE.exe 생성
  - shaders POST_BUILD 복사 정상 (build/Debug/shaders/TriangleVS.hlsl, TrianglePS.hlsl)
- 실행 결과: build/Debug/CHALKFORGE.exe 약 4초 실행 후 정상 종료(무크래시, ExitCode=0)
- 로그 결과(`build/Debug/chalkforge.log`, 오류 없음):
  - [INFO] CHALKFORGE initializing
  - [INFO] D3D12 debug layer enabled.
  - [INFO] Depth buffer created (1280x720). ×3
  - [INFO] Mesh created (24 vertices, 36 indices, 1 submeshes).
  - [INFO] Material created (256 byte constant buffer, 48 byte payload). ×2
  - [INFO] D3D12 graphics initialized (1280x720).
  - [INFO] ChalkMesh released. / ChalkMaterial released. ×2 / ChalkResource released. ×3
  - [INFO] CHALKFORGE shutting down
- 자동 검증 완료: 빌드, 실행, 초기화/해제 로그, 씬 메시/머티리얼 생성 로그
- 미검증(사용자 확인 대기): 육안 렌더(씬 큐브 2개 + 방향성 조명) 정상 표시 여부
- 미검증: D3D12 debug layer validation 경고/오류는 OutputDebugString으로 출력되어 파일 로그에 캡처되지 않음 → 로그상 오류 없음은 스모크 테스트 수준
- 미구현(범위): 카메라 이동(WASD/마우스 궤도) 입력 미연결 (정적 카메라 사용)

### [PHASE 05 STEP G] - 2026-10-19
- 빌드 결과: Debug/Release 빌드 성공 (Step F와 동일 산출물, 문서화 단계로 코드 변경 없음)
- 실행 결과: build/Debug/CHALKFORGE.exe 정상 실행(무크래시)
- 로그 결과: 초기화/메시·머티리얼 생성/정상 종료 로그, D3D12 오류 없음
- **사용자 육안 확인 완료(사용자 보고 "큐브 두 개 나온다")**: 씬 큐브 2개(CenterCube/OffsetCube)가 화면에 정상 표시됨
- 판정: PHASE 05의 렌더 통합(카메라/메시/머티리얼/씬/조명) 검증 완료
- 잔여(후속 작업): 카메라 이동(WASD/마우스 궤도) 입력 연결 미구현(정적 카메라), D3D12 debug layer validation 메시지 파일 로그 미캡처

### [PHASE 05 STEP H] - 2026-10-19
- 빌드 결과: Debug/Release 빌드 성공 (경고 없음)
  - `cmake --build build --config Debug` → build/Debug/CHALKFORGE.exe 생성
  - `cmake --build build --config Release` → build/Release/CHALKFORGE.exe 생성
- 실행 결과: build/Release/CHALKFORGE.exe 4초 이상 실행 유지 후 정상 종료(무크래시)
- 로그 결과(오류 없음): [INFO] CHALKFORGE initializing / D3D12 debug layer enabled / Depth buffer created (1280x720) ×3 / Mesh created (24 vertices, 36 indices, 1 submeshes) / Material created (256 byte constant buffer, 48 byte payload) ×2 / D3D12 graphics initialized (1280x720)
- 자동 검증 완료: 빌드, 실행 유지, 초기화 로그
- 미검증(사용자 확인 대기): 입력 동작 — W/S/A/D 이동, 마우스 좌클릭 드래그 궤도 회전. 합성 입력 주입이 불가하여 자동 검증 불가
- 주의: 스모크 테스트 로그는 Start-Process가 작업 디렉터리를 상속하여 루트 `chalkforge.log`에 기록됨(`build/Release/chalkforge.log`가 아님)

### 오류 해결 기록
- C4244 (wide→narrow 변환 경고): ChalkGraphics.cpp 익명 네임스페이스에 로컬 WideToUtf8(wstring→UTF-8) 헬퍼 추가, 셰이더 경로 변환에 적용 → 경고 제거
- 링커 오류: d3d12/dxgi/d3dcompiler 라이브러리 링크 누락 → CMakeLists.txt target_link_libraries에 추가로 해결
- 한글 경로 콘솔 출력 깨짐: MSVC /utf-8 컴파일 옵션 추가로 해결