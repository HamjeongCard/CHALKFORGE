PHASE 02 — Windows Application & Window Management

**목표**: 완전한 창 관리 시스템과 입력 처리

**현재 구현 상태**: 미구현 (Not Implemented)

**확인된 상태**:
- CMakeLists.txt: 현재 src/ChalkApp.cpp 하나만 포함, PHASE 02 소스 미추가
- src/ChalkApp.cpp: 콘솔 애플리케이션으로만 동작, Win32 창 미생성
- 프로젝트 빌드: 성공 (Release/Debug 모두), 하지만 콘솔 출력만 수행
- 실행: CHALKFORGE 텍스트 콘솔 출력만 확인, 윈도우 생성 안 됨

**미완성 항목** (완료로 기록하지 않음):
- Win32 창 생성 및 관리 (ChalkWindow.h/cpp 미구현)
- 메시 처리 (WM_SIZE, WM_MOVE, WM_PAINT 미구현)
- DPI 인식 설정 미구현
- 여러 모니터 기본 지원 미구현
- D3D12 스왑 체인용 창 크기 조정 처리 미구현
- 키보드 메시 처리 미구현
- 마우스 메시 처리 (추적 상태) 미구현
- 이벤트 시스템 구조체 기반 큐 미구현

**로드맵 PHASE 02 목표와 현재 격차**:
| 항목 | PHASE 02 로드맵 목표 | 현재 상태 |
|------|---------------------|-----------|
| Win32 창 생성 | Complete Win32 window creation & management | ❌ Not started |
| 메시 처리 | Message handling (WM_SIZE, WM_MOVE, WM_PAINT) | ❌ Not started |
| DPI 인식 | DPI awareness settings | ❌ Not started |
| 창 크기 조정 | D3D12 swap chain resize handling | ❌ Not started |
| 입력 메시지 | Keyboard & mouse message handling | ❌ Not started |
| 이벤트 시스템 | Event structure-based event queue | ❌ Not started |

**기술적 위험**: 창 크기 조정으로 인해 D3D12 리소스 재생성 문제 (미구현이므로 현재 미해결)

**학습 목표**: Win32 API 기초, 메시 펌프, DPI 인식, 창 크기 조정 로직 (미구현)

**검증 절차**: 창 생성, 크기 조정 처리, 입력 메시지 처리 (현재 진행 전)

**수락 기준**: 적절한 DPI 처리 기능이 있는 크기 조정 가능한 창, 이벤트 시스템 기능적 (현재 미충족)

**미해결 이슈**: 
- PHASE 02 구현을 위한 ChalkWindow 소스 파일 필요
- CMakeLists.txt에 새 소스 파일 추가 필요
- WinMain 및 창 절차 구현 필요