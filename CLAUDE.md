# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## 프로젝트 개요

**Pentagram** — Unreal Engine 5.7 기반 쿼터뷰 다크 판타지 액션 RPG.  
중세 배경에서 악마·언데드·크리쳐를 상대하는 핵앤슬래시 전투가 핵심이며, 디아블로 스타일 쫄몹 전투 + Lost Ark 스타일 패턴 보스 전투를 지향한다.

---

## 빌드 및 개발 명령

### 프로젝트 생성 / 빌드

```powershell
# Visual Studio 프로젝트 파일 재생성 (에디터 실행 전 필요시)
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="C:\Unreal\7th-Team4-Final-Project\Pentagram.uproject" -game -rocket -progress

# Development Editor 빌드 (UnrealBuildTool 직접)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" PentagramEditor Win64 Development "C:\Unreal\7th-Team4-Final-Project\Pentagram.uproject" -waitmutex

# Shipping 빌드
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" Pentagram Win64 Shipping "C:\Unreal\7th-Team4-Final-Project\Pentagram.uproject" -waitmutex
```

실제 개발 중에는 Visual Studio(`.sln`)에서 **Development Editor** 구성으로 빌드하거나, Unreal Editor 내 **컴파일** 버튼을 사용한다.

### 핫 리로드 / Live Coding

에디터 실행 중 C++ 변경은 `Ctrl+Alt+F11` (Live Coding) 또는 에디터 하단 **Compile** 버튼으로 반영한다.

---

## 아키텍처 구조

### 모듈

소스는 단일 런타임 모듈 `Pentagram`으로 구성된다 (`Source/Pentagram/`).

```
Source/Pentagram/
├── PentagramCharacter         ← 쿼터뷰 캐릭터 베이스 (SpringArm 카메라 포함)
├── PentagramGameMode          ← 게임 모드 베이스 (스텁)
├── PentagramPlayerController  ← 포인트-앤-클릭 이동 + NavMesh 경로 추적
├── Variant_Strategy/          ← RTS 스타일 전략 게임 변형
│   ├── StrategyGameMode
│   ├── StrategyPawn            (카메라 전용 플로팅 폰)
│   ├── StrategyPlayerController (박스 선택, 줌, 터치/마우스 이중 지원)
│   ├── StrategyUnit            (AI 제어 유닛)
│   └── UI/
└── Variant_TwinStick/         ← 트윈스틱 슈터 변형
    ├── TwinStickCharacter      (회전-조준, 발사체/AoE 공격, 대시, 낙백)
    ├── TwinStickGameMode       (점수·콤보 배율·스폰 상한 관리)
    ├── TwinStickPlayerController
    ├── AI/
    │   ├── TwinStickAIController (StateTree 컴포넌트 기반)
    │   ├── TwinStickNPC          (점수·드랍 포함 적)
    │   ├── TwinStickSpawner
    │   └── TwinStickStateTreeUtility
    ├── Gameplay/
    │   ├── TwinStickProjectile  (바운스 발사체)
    │   ├── TwinStickAoEAttack   (지속 범위 데미지 존)
    │   └── TwinStickPickup
    └── UI/
        └── TwinStickUI
```

### 주요 의존 모듈 (Build.cs)

`EnhancedInput`, `AIModule`, `NavigationSystem`, `StateTreeModule`, `GameplayStateTreeModule`, `Niagara`, `UMG`, `Slate`

### 플러그인

| 플러그인 | 용도 |
|---|---|
| StateTree | NPC AI 행동 트리 정의 |
| GameplayStateTree | StateTree AI 컴포넌트 연동 |
| ModelingToolsEditorMode | 에디터 전용 모델링 툴 |

---

## 핵심 시스템

### 입력

EnhancedInput 시스템 사용. 마우스/키보드, 게임패드, 터치(모바일) 세 가지 매핑 컨텍스트를 분리해서 관리한다. `APentagramPlayerController`에서 런타임에 컨텍스트를 전환한다.

### AI

`UStateTreeAIComponent`를 `ATwinStickAIController`에 붙여 행동 로직을 정의한다. `TwinStickStateTreeUtility`는 StateTree 태스크에서 호출하는 정적 유틸리티 함수를 제공한다.

### 콤보·점수 (TwinStick)

`ATwinStickGameMode`가 전역 점수와 콤보 배율(최대 4x)을 관리한다. 마지막 킬 후 3초 경과 시 콤보 초기화. NPC 스폰 상한은 기본 20.

### 카메라

`APentagramCharacter` 베이스에 `USpringArmComponent` + `UCameraComponent` 조합. 쿼터뷰 고정 시점을 기본으로 한다.

---

## 개발 규칙

- **기본 언어**: 한국어 (주석, 커밋 메시지, 문서 모두 한국어 우선)
- 새 게임플레이 클래스는 `Variant_*` 하위 폴더에 추가하고, 베이스 클래스(`APentagramCharacter` 등)를 상속한다.
- Blueprint 구현이 주가 되며 C++은 기반 로직만 제공하는 구조를 유지한다.
- UPROPERTY/UFUNCTION 카테고리 메타데이터를 항상 명시한다.
