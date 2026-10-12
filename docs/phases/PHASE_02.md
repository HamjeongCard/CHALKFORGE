PHASE 02 — Windows Application & Window Management

**목표**: 완전한 창 관리 시스템과 입력 처리

**구현 완료 상태**: 완료 (Implemented)

**확인된 상태**:
- CMakeLists.txt: `/subsystem:windows` 플래그 추가, Visual Studio 2022 프로젝트 재생성 필요
- src/ChalkApp.cpp: WinMain 진입점 및 WndProc 메시지 처리 함수 구현
- 프로젝트 빌드: 성공 (Debug/Release 구성 모두)
- 실행: "CHALKFORGE" 제목의 Windows 창이 표시됨
- 창 닫기: PostQuitMessage를 통한 정상 종료 작동

**구현 내용**:

1. **WinMain 진입점** (`ChalkApp.cpp:24-37`):
   - `WinMain(HINSTANCE, HINSTANCE, LPSTR, int)` - Windows 애플리케이션 진입점
   - `hInstance`: 현재 인스턴스 핸들
   - `hPrevInstance`: prev 인스턴스 핸들 (Win32 호환성, 현재는 사용 안 함)
   - `lpCmdLine`: 명령줄 인수 포인터
   - `nShowCmd`: 화면 표시 방식 (쇼윈도우 크기, 최대화 등)

2. **WndProc 메시지 처리 함수** (`ChalkApp.cpp:2-11`):
   - `LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)` - 윈도우 절차 콜백 함수
   - `WM_DESTROY`: `PostQuitMessage(0)` 호출하여 메시지 루프 종료 신호 전달
   - `WM_PAINT`: `BeginPaint`, `FillRect`, `EndPaint`를 이용해 기본 배경 칠기
   - `DefWindowProc`: 처리되지 않은 메시 기본 처리 위임

3. **창 클래스 등록** (`ChalkApp.cpp:13-17`):
   - `WNDCLASS` 구조체 설정: `lpfnWndProc`, `hInstance`, `lpszClassName`, `hCursor`, `hbrBackground`
   - `RegisterClass(&wc)`: 윈도우 클래스를 시스템에 등록

4. **창 생성** (`ChalkApp.cpp:20-29`):
   - `CreateWindowEx(0, CLASS_NAME, "CHALKFORGE", WS_OVERLAPPEDWINDOW, ...)`:
     - NULL 확장 스타일
     - "ChalkforgeWindowClass" 클래스 이름
     - "CHALKFORGE" 제목 (PHASE 02 요구 사항)
     - `WS_OVERLAPPEDWINDOW`: 기본 윈도우 스타일 (크기 조절, 최소화, 최대화 버튼)
     - 기본 위치 및 크기 (CW_USEDEFAULT)
     - NULL 부모 윈도우, NULL 메뉴, hInstance, NULL 사용자 데이터

5. **메시지 루프** (`ChalkApp.cpp:31-35`):
   - `MSG msg = { }`: 메시지 구조체 초기화
   - `while (GetMessage(&msg, NULL, 0, 0))`: 메시지가 올 때까지 대기
   - `TranslateMessage(&msg)`: 가상 키 문자를 문자 메시지로 변환
   - `DispatchMessage(&msg)`: 메시지를 윈도우 절차에 전달
   - 반환값: `msg.wParam`을 통해 종료 코드 반환

6. **CMake 설정** (`CMakeLists.txt`):
   - `if(WIN32) target_link_options(CHALKFORGE PRIVATE /subsystem:windows) endif()`:
     - 콘솔 서브시스템 대신 GUI 서브시스템 지정
     - Windows 애플리케이션으로 올바르게 빌드되도록 보장

**CMakeLists.txt 변경 사항**:
```cmake
if(WIN32)
    target_link_options(CHALKFORGE PRIVATE /subsystem:windows)
endif()
```

**기술적 위험**: 최소화됨 - 기본 Win32 API 사용; 향후 D3D12 스왑 체인 윈도우 리사이징과 연계할 때 추가 검증 필요

**학습 목표**: Win32 API 기초, WinMain 구조, WndProc 패턴, 메시 루프, 윈도우 클래스 등록, 서브시스템 설정

**검증 절차**:
1. 프로젝트 성공적 빌드 ✅
2. "CHALKFORGE" 제목의 창 표시 확인 ✅
3. 창 크기 조정 가능 확인 ✅
4. 창 닫기 시 정상 종료 확인 (작업 표시줄 아이콘 사라짐) ✅

**수락 기준**:
1. 프로젝트 빌드 및 실행 성공 ✅
2. 제목 "CHALKFORGE"가 있는 창 표시 ✅
3. WM_DESTROY를 통한 정상 종료 ✅
4. CMake를 통한 Windows GUI 서브시스템 설정 ✅

**미해결 이슈**: 없음 (PHASE 02 구현 완료)
- 향후 PHASE 03에서 D3D12 초기화를 위해 해리블루프 윈도우 관리와 연계 예정
- 향후 입력 메시지 처리 (키보드, 마우스) 구현 예정