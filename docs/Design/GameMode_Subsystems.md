# 게임모드 & 서브시스템 설계서 — MVP

**프로젝트** : 악마 창궐 세계관 핵&슬래시 RPG
**문서 버전** : v0.2 (MVP)
**담당자** : 정호승
**작성일** : 2026-05-15
**엔진** : Unreal Engine 5.7 / C++

---

## 목차

1. [게임모드 시스템](#1-게임모드-시스템)
2. [GameInstance 서브시스템](#2-gameinstance-서브시스템)

---

# 1. 게임모드 시스템

## 1.1 개요

게임의 전체 흐름(씬 전환, 승패 판정, 플레이어 스폰, 상태 관리)을 담당하는 시스템.

**책임 범위**
- 게임 상태(State) 전환 및 관리
- 플레이어 스폰 / 리스폰 처리
- 클리어 / 게임오버 판정
- 챕터 진입·종료 흐름 제어
- 경험치·드랍 분배 트리거

## 1.2 GameMode / GameState 역할 분리 원칙

> **핵심 규칙**: `GameMode`는 **서버 전용**. 클라이언트에 알릴 때는 반드시 `GameState`를 경유한다.

| 주체 | 존재 위치 | 역할 |
|------|---------|------|
| `ABaseGameMode` | 서버 전용 | 판정·분배 등 게임 로직 처리 |
| `ABaseGameState` | 서버 + 클라이언트 복제 | 상태 전파, UI 이벤트 전달 |

- **흐름**: `GameMode(서버)` → `GameState(서버)` → `GameState(클라이언트)` → UI
- `GameMode`에서 직접 Multicast / Client RPC 호출 금지

## 1.3 게임 흐름

```
메인 메뉴 →  챕터 입장
  → 일반 구역 → 상인 구역 → 보스 구역
  → 보스 처치
```

## 1.4 게임 상태 (EGamePhase)

| 상태 | 전환 조건 |
|------|---------|
| `Playing` | 기본 상태 |
| `BossFight` | 보스방 진입 |
| `GameOver` | 파티 전원 사망 |

- 상태 전환 판정은 `GameMode` → `GameState.CurrentPhase` 갱신 → 클라이언트 복제 → `OnGamePhaseChanged` 브로드캐스트 → UI 수신

## 1.5 승패 조건 및 데스 페널티

- **게임오버**: 파티 전원 사망(보스전에만 해당)

| 조건 | 페널티 |
|------|--------|
| 개인 사망 | 경험치 일정량 차감 |
| 전원 사망 | 경험치 일정량 차감 |

## 1.6 리스폰

- 개인 사망 → 마지막 등록 세이브 포인트 앞 즉시 리스폰
- 세이브 포인트 위치는 서버 관리
- 전원 사망 시 보스 진입 전 세이브 포인트로 리스폰(보스전에만 해당)

## 1.7 챕터 구성 (1챕터 - 광산)

> 구역 배치·레이아웃은 레벨 디자인 담당. GameMode는 진입/클리어 감지 및 상태 전환만 담당.

| 구역 | GameMode 처리 | 트리거 |
|------|-------------|--------|
| 일반 구역 | 상태 전환 없음 (`Playing` 유지) | - |
| 보스 구역 | `Playing` → `BossFight` | BoxCollider 진입 이벤트 |

## 1.8 클래스 구조

```cpp
ABaseGameMode (서버 전용)
├── void SetGamePhase(EGamePhase)
├── void OnBossDefeated()
├── void OnAllPlayersDead()
└── void RespawnPlayer(APlayerController*)

ABaseGameState (서버 + 클라이언트 복제)
├── UPROPERTY(ReplicatedUsing=OnRep_GamePhase)
│   EGamePhase CurrentPhase
├── void OnRep_GamePhase()
├── FOnGamePhaseChanged OnGamePhaseChanged
├── int32 AlivePlayerCount
```

## 1.9 작업 분해 (MVP)

| # | 작업 | 선행 | 산출물 |
|---|------|------|--------|
| 1 | 게임 상태 열거형 정의 (EGamePhase) | - | GameTypes.h |
| 2 | BaseGameMode 클래스 구현 | #1 | BaseGameMode |
| 3 | BaseGameState 클래스 구현 | #1 | BaseGameState |
| 4 | 플레이어 스폰 / 리스폰 로직 | #2 + 플레이어 | GameMode 내 |
| 5 | 게임오버 판정 | #2 + 파티 시스템 | GameMode 내 |
| 6 | 경험치·드랍 분배 처리 | #2 + 아이템/파티 | GameMode 내 |

---

# 2. GameInstance 서브시스템

## 2.1 개요

게임을 껐다 켜도, 챕터를 이동해도 유지되어야 하는 데이터를 관리하는 서브시스템 모음.
모두 `UGameInstanceSubsystem` 기반으로 구현한다.

| 서브시스템 | 클래스명 | 역할 |
|-----------|---------|------|
| 경제 | `UEconomySubsystem` | 골드 보유량 관리, 수입/지출 처리 |
| 플레이어 진행 | `UPlayerProgressSubsystem` | 레벨, 경험치 관리 |
| 세이브 | `USaveSubsystem` | 위 데이터 전체 저장·불러오기 |

**역할 분리 원칙**
- 각 서브시스템은 자기 데이터만 관리
- 세이브 서브시스템은 나머지에서 데이터를 수집해 저장·복원만 담당
- HP/MP 등 전투 중 수치는 캐릭터 시스템에서 관리

---

## 2.2 경제 서브시스템 (UEconomySubsystem)

### 관리 데이터

| 데이터 | 타입 | 설명 |
|--------|------|------|
| `CurrentGold` | `int32` | 현재 보유 골드 |

### 주요 기능

| 함수 | 설명 |
|------|------|
| `AddGold(int32 Amount)` | 골드 획득 및 소비 |
| `GetCurrentGold()` | 현재 골드 조회 |
| `CanAfford(int32 Amount)` | 구매 가능 여부 확인 |

### 이벤트

| 이벤트 | 발생 시점 | 구독 대상 |
|--------|---------|---------|
| `OnGoldChanged(int32 NewAmount)` | 골드 증감 시 | UI 골드 표시 |

- 멀티플레이 골드 — **개인별 관리** (확정)

---

## 2.3 플레이어 진행 서브시스템 (UPlayerProgressSubsystem)

### 관리 데이터

| 데이터 | 타입 | 설명 |
|--------|------|------|
| `PlayerLevel` | `int32` | 현재 레벨 |
| `CurrentExp` | `int32` | 현재 경험치 |

### 주요 기능

| 함수 | 설명 |
|------|------|
| `AddExp(int32 Amount)` | 경험치 획득 — 레벨업 조건 충족 시 자동 레벨업 |
| `GetCurrentLevel()` | 현재 레벨 조회 |
| `GetCurrentExp()` | 현재 경험치 조회 |
| `ApplyDeathPenalty()` | 사망 페널티 — 경험치 차감 |

---

## 2.4 세이브 서브시스템 (USaveSubsystem)

### 저장 데이터 구조 (FSaveData)

| 데이터 | 타입 | 출처 |
|--------|------|------|
| `PlayerLevel` | `int32` | UPlayerProgressSubsystem |
| `CurrentExp` | `int32` | UPlayerProgressSubsystem |
| `CurrentGold` | `int32` | UEconomySubsystem |
| `InventoryItems` | `TArray<FName>` | 아이템 시스템 |
| `EquippedItems` | `TMap<FName, FName>` | 아이템 시스템 |

### 주요 기능

| 함수 | 설명 |
|------|------|
| `SaveGame()` | 현재 상태를 슬롯에 저장 |
| `LoadGame()` | 저장된 데이터 불러와 각 서브시스템에 복원 |
| `HasSaveData()` | 저장 데이터 존재 여부 확인 |
| `DeleteSaveData()` | 저장 데이터 삭제 |

### 자동 저장 시점

| 시점 | 트리거 |
|------|--------|
| 구역 이동 시 | 포탈 / 구역 전환 트리거 통과 시 |
| 앱 종료 | 종료 버튼 선택 시 |

### 데이터 수집 흐름

```
SaveGame() 호출
    ↓
각 서브시스템 / 시스템에서 데이터 수집
    ├── UPlayerProgressSubsystem  →  레벨, 경험치
    ├── UEconomySubsystem         →  골드
    ├── 스킬 시스템               →  해금 스킬, 슬롯 배치
    └── 아이템 시스템             →  인벤토리, 장착 장비
    ↓
FSaveData 조립 → USaveGame 슬롯에 저장
```

---

## 2.5 작업 분해 (MVP)

| # | 작업 | 선행 | 산출물 |
|---|------|------|--------|
| 1 | FSaveData 구조체 정의 | 각 담당자 데이터 항목 확정 | SaveData.h |
| 2 | UEconomySubsystem 구현 | - | EconomySubsystem.cpp |
| 3 | UPlayerProgressSubsystem 구현 | - | PlayerProgressSubsystem.cpp |
| 4 | 레벨업 로직 및 이벤트 구현 | #3 + 캐릭터 시스템 | PlayerProgressSubsystem 내 |
| 5 | USaveSubsystem 구현 | #1 | SaveSubsystem.cpp |
| 6 | 각 서브시스템 데이터 수집 연동 | #5 + #2 + #3 | SaveSubsystem 내 |
| 7 | 스킬·아이템 데이터 수집 연동 | #5 + 각 담당자 API | SaveSubsystem 내 |
| 8 | 챕터 클리어 자동 저장 연동 | #5 + GameMode | SaveSubsystem 내 |
| 9 | 게임 시작 시 로드·복원 흐름 | #5 + 전체 서브시스템 | SaveSubsystem 내 |

---

## 3. 네이밍 규칙

| 종류 | 규칙 | 예시 |
|------|------|------|
| GameMode 클래스 | `A{Domain}GameMode` | `ABaseGameMode` |
| GameState 클래스 | `A{Domain}GameState` | `ABaseGameState` |
| 서브시스템 클래스 | `U{Domain}Subsystem` | `UEconomySubsystem` |
| 구조체 | `F{Domain}Data` | `FSaveData` |
| 열거형 | `E{Domain}` | `EGamePhase` |
| 델리게이트 | `FOn{Event}` | `FOnGamePhaseChanged`, `FOnLevelUp` |
| 폴더 | `Source/{Project}/GameMode/`, `Source/{Project}/Subsystems/` | |

---

## 4. 결정이 필요한 항목

- 챕터 재시작 시 골드 — **유지** (확정)
- 레벨업 필요 경험치 공식 확정 (캐릭터 담당 협의)
- 아이템 식별자 타입 — `FName` vs `int32` ID (아이템 담당 협의)
- 장비 슬롯 구조 — 슬롯 키 네이밍 확정 필요 (아이템 담당 협의)
- 멀티플레이 최대 인원 수 확정

---

## 5. 확장 포인트 (MVP 이후)

- 챕터 2, 3 추가 및 챕터 선택 로비
- 인원 수에 따른 몬스터 스탯 자동 보정
- 멀티 세이브 슬롯 / 수동 저장
- 퀘스트 진행 상황 저장
- 클리어 타임 기록 / 랭킹
- 클라우드 세이브 연동

---

## 일정표

| 작업 | 설명 | 예상 기간 |
|------|------|----------|
| APTGameMode | 서버 전용. 게임 상태 전환, 리스폰, 승패 판정, 경험치·드롭 분배 | 3일 |
| APTGameState | 서버 + 클라이언트 복제 | 2일 |
| UPTEconomySubsystem | 골드 보유량 관리, 수입/지출 처리 (개인별) | 1일 |
| UPTPlayerProgressSubsystem | 레벨, 경험치 관리, 레벨업 이벤트 발행 | 2일 |
| UPTSaveSubsystem | 골드·레벨·인벤토리·장비·스킬 슬롯 전체 저장·불러오기 | 2~3일 |
