LEARNING_LOG.md - CHALKFORGE 학습 로그
==========================================

## Phase 00 - 2026-10-12
### 환경 감사
- **MSVC 14.44.35207** (Visual Studio 2022 Community): full Direct3D 12 지원 C++ 컴파일러
- **Windows Kits 10.0.26100.0** (Windows 11 SDK) 및 10.0.22621.0 (Windows 10 22H2): D3D12 헤더 (d3d12.h, d3d12shader.h), 라이브러리 (d3d12.lib, d3dcompiler.lib)
- **Direct3D 12 전체 기능 세트**: 디바이스 생성, 명령 큐, 스왑 체인, 기술 힙, 루트 서명, 파이프라인 상태 객체, 펜싱, 리소스 바리어
- **CMake**: 빌드 시스템 생성을 위해 사용 가능

### 배운 Direct3D 12 개념
- **CPU vs GPU 역할**: CPU는 명령 목록/버퍼 준비; GPU는 명령 큐에서 명령 실행
- **D3D12 디바이스**: 주요 추상화; D3D12CreateDevice(); adapter 기능 대표
- **명령 큐**: 명령 목록 제출 지점; 유형은 direct (그래픽) 또는 compute 지정
- **명령 목록**: 렌더링 명령 기록; command allocator로부터 생성되어야 함
- **명령 할당자**: 명령 목록을 위한 메모리; 각 프레임마다 적절히 재활용을 위해 Reset
- **기술 힙**: CPU가 보이는 힙으로, RTV, DSV, CBV/SRV/UAV 핸들 포함
- **루트 서명**: 셰이더가 볼 수 있는 파라미터 (루트 Constants, descriptor, samplers) 정의
- **파이프라인 상태 객체 (PSO)**: 완전한 GPU 상태 (셰이더 + 입력 레이아웃 + 라스터화자 + 깊이/스텐실 + 블렌딩)
- **리소스 바리어**: 리소스 상태 전환 (렌더 타겟, 복사 등 사이의 전환 바리어)
- **스왑 체인**: 렌더된 버전을 윈도우에 제시; 뒷 버퍼 관리 담당
- **펜싱**: GPU-CPU 동기화 객체; 명령 완료 신호 및 대기
- **버텍스/인덱스 버퍼**: GPU에 Geometry 데이터 저장; Resource heap을 통한 업로드
- **HLSL 셰이더**: .hlsl에서 .cso로 또는 런타임 컴파일 변환된 버텍스 + 픽셀 셰이더
- **깊이 버퍼 (DSV)**: 깊이 테스트 사용 가능; 멀리 있는 기하학이近处 기하학을 가리는 것 방지
- **더블/트리플 버퍼링**: 프레임 파이프라인 평행성을 위한 스왑 체인 버퍼 수

### 아키텍처 설계 원칙
- **모듈식 분해**: 코어/플랫폼/그래픽스/씬/보드/그리기/도구/편집/UI/지속성 간의 명확한 경계
- **명시적 리소스 관리**: GPU 리소스 수명 추적; 프레임당 명령 할당자 재활용
- **펜싱 기반 동기화**: GPU/CPU 동기화용 펜싱; 트리플 버퍼링 패턴
- **CPU 측 데이터와 GPU 측 리소스 분리**: CPU 측 스트로크 데이터; GPU 측 기하학 버퍼; Map/Unmap 또는 매핑된 상태로 업데이트
- **툴 확장성 인터페이스**: 핵심 수정 없이 새로운 드로잉 툴 추가를 위한 추상 툴 기본 클래스
- **Undo/Redo를 위한 명령 패턴**:fine-grained 역사 제어를 위한 명시적 명령 유형과 페이로드
- **버전화된 데이터 형식**: backward 호환성과 마이그레이션을 위한 형식 버전 번호

### 참고할 주요 파일 및 패턴
- D3D12 디바이스 생성: `D3D12CreateDevice()`
- 스왑 체인: `CreateSwapChainForHwnd()`
- 명령 할당자/목록: `Reset()`, `Close()`, `Open()`, `Reset()`, `RecordCommands()`
- 기술 힙: `CreateDescriptorHeap()`, `GetCPUDescriptorHandleForHeapStart()`
- 루트 서명: `CreateRootSignature()`
- PSO: `CreatePipelineState()`
- 펜싱: `CreateFence()`, `SetSignalValue()`, `SetEventOnCompletion()`, `WaitForSingleObject()`
- 리소스 바리어: `ResourceBarrier()`, `Transition()`
- 버텍스 버퍼: CreateCommittedResource + UploadBuffer 패턴
- Constant 버퍼: 256-byte 경계 맞춤; `CreateConstantBufferView()`

### 향후 단계별 학습 주제
- 멀티샘플 품질 레벨 및 샘플 설명
- 스왑 체인 색상 공간 처리 (sRGB)
- 고급 리소스 상태 (레이 트레이싱, 메쉬 셰이더)
- 비동기 명령 목록 기록
- 그림자 맵핑 및 간접 그리기
- GPU 기반 렌더링 패턴

### 검토 진행 상황
- D3D12 개발을 위한 환경 완전히 검증
- 6대 주요 시스템 카테고리 전반에 걸쳐 아키텍처 문서화
- 기초부터 확장성까지 15단계 로드맵 정의
- 모든 핵심 선택에 대한 결정 근거 기록
- 다음 학습 초점: PHASE 01 구현 패턴, CMake 프로젝트 설정, Win32 애플리케이션 기초