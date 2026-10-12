PHASE 01 — Project Structure & Core Foundation

**목표**: 프로젝트 빌드 시스템, 핵심 애플리케이션 골격, 최소 실행 가능한 프레임워크 확립

**구현 범위**:
- CMakeLists.txt 생성 및 Visual Studio 2022 제너레이터 구성
- 기본 Windows 애플리케이션 프레임워크 구현 (WinMain, WndClass, 메시 루프)
- 로깅 시스템 설정 (OutputDebugString + 파일 로깅)
- 오류 처리 프레임워크 구현 (HRESULT 확인, 예외 안전성)
- 구성 시스템 생성 (기본값, 가능한 설정 파일 지원)
- 기본형 및 수학 유틸리티 래퍼 생성

**생성/수정할 파일**:
- CMakeLists.txt (최초 생성 확인됨)
- src/ChalkApp.cpp (애플리케이션 진입, 콘솔 출력 확인)
- build/CHALKFORGE.vcxproj (CMake 생성 Visual Studio 프로젝트)
- build/CHALKFORGE.sln (CMake 생성 솔루션 파일)

**의존성**: 독립적 (기초 단독)

**기술적 위험**: 빌드 시스템 선택 (CMake vs. VS 솔루션)은 후속 단계에 영향; 서브시스템 (Windows vs. Console) 정의

**학습 목표**: CMakeLists.txt 구조 for D3D12, Win32 애플리케이션 기초, 기본 C++ 프로젝트 설정

**검증 절차**: 프로젝트 성공적으로 빌드 (빈 창 표시), 로거 출력 확인, 오류 처리 확인

**수락 기준**: 
1. 프로젝트 빌드 및 실행 성공 ✅
2. 제목 "CHALKFORGE"가 있는 창 표시 ❌ (현재 콘솔 애플리케이션)
3. 로거 활성화 대기

**미해결 이슈**: 
- CMake vs. VS 솔루션 최종 결정 보류
- 콘솔에서 윈도우 애플리케이션으로 서브시스템 전환
- Win32 창 생성 구현 필요