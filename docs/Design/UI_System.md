# UI 시스템 설계서 — MVP

**프로젝트** : 악마 창궐 세계관 핵&슬래시 RPG
**문서 버전** : v0.6 (MVP)
**담당자** : 문인우
**작성일** : 2026-05-15
**엔진** : Unreal Engine 5.7 / C++ + Blueprint

---

## 1. 개요

핵&슬래시 RPG의 UI 시스템 MVP 설계 요약. 다른 시스템과는 이벤트 기반의 느슨한 결합을 유지한다.

### 1.1 UI 매니저 책임 범위

- 위젯 생성/소멸 및 레이어 관리
- 입력 모드(Game / UI / GameAndUI) 자동 전환
- 화면 전환 효과(페이드 In/Out)

### 1.2 구현 방식

- UMG + CommonUI 플러그인
- C++ 베이스 + WBP 파생
- UI는 데이터를 직접 읽지 않고 이벤트만 구독

---

## 2. UI 화면 목록

| 화면 | 최소 기능 | 연관 시스템 |
|---|---|---|
| HUD | HP/MP, 레벨/경험치, 스킬 슬롯+쿨타임, 골드 | 캐릭터 / 스킬 / 경제 |
| 인벤토리 / 장비창 | 그리드, 장착 슬롯, 툴팁 | 아이템 / 캐릭터 |
| 일반몹 체력바 | 월드 부착 HP바 | 전투 |
| 보스 체력바 | 상단 노출, 페이즈 표시 | 전투 |
| 스킬 창 | 스킬 목록, 슬롯 배치 | 스킬 |

---

## 3. 아키텍처

| 구분 | 내용 |
|---|---|
| 진입점 | `ULocalPlayerSubsystem` (`UPTUIManagerSubsystem`) |
| 베이스 위젯 | `UUserWidgetBase` (`UPTUserWidgetBase`) |
| 화면별 위젯 | 베이스 상속 C++ + WBP |
| 통신 | Delegate 단방향 (시스템 → UI) |
| 플러그인 | CommonUI |

---

## 4. 화면별 기능 요약

### 4.1 HUD

| 항목 | 내용 |
|---|---|
| 위치 | 상시 (HUD) |
| 구성 | HP/MP, Lv/Exp, 골드, 스킬 슬롯 4개 + 쿨타임 |
| 동작 | 이벤트 수신 시 해당 위젯만 갱신 |
| 이벤트 | HP/MP/Lv/Exp 변경, 스킬 쿨타임/슬롯, 골드 변경 |
| 클래스 | `UPTHUDWidget`, `UPTPlayerStatusWidget`, `UPTSkillSlotPanelWidget`, `UPTSkillSlotWidget` |

### 4.2 인벤토리 / 장비창

| 항목 | 내용 |
|---|---|
| 위치 | Window (I 토글) |
| 구성 | 5×6 그리드, 장착 슬롯 2개, 툴팁 |
| 동작 | 마우스오버 → 툴팁, 우클릭 → 사용/장착, 드래그&드롭 |
| 이벤트 | 인벤토리/장비 변경 |
| 클래스 | `UPTInventoryWidget`, `UPTInventoryGridWidget`, `UPTInventorySlotWidget`, `UPTEquipmentSlotWidget`, `UPTItemTooltipWidget` |

### 4.3 일반몹 체력바

| 항목 | 내용 |
|---|---|
| 위치 | 몹 액터 부착 (WidgetComponent, World) |
| 구성 | HP 바 |
| 동작 | 피격 시 노출, 일정 시간 후 자동 숨김, 빌보드 |
| 이벤트 | 몹 HP 변경 |
| 클래스 | `UPTMonsterHealthBarWidget` |

### 4.4 보스 체력바

| 항목 | 내용 |
|---|---|
| 위치 | 화면 상단 중앙 (HUD) |
| 구성 | HP 바, 페이즈 세그먼트, 보스명 |
| 동작 | 등장 시 노출, 페이즈 변화 시 갱신, 사망 시 숨김 |
| 이벤트 | 보스 스폰/페이즈/사망 |
| 클래스 | `UPTBossHealthBarWidget` |

### 4.5 스킬 창

| 항목 | 내용 |
|---|---|
| 위치 | Window (K 토글) |
| 구성 | 스킬 목록, 슬롯 4개, 쿨타임·MP 표시 |
| 동작 | 리스트 → 슬롯 클릭 배정 |
| 이벤트 | 스킬 슬롯 변경, 쿨타임 |
| 클래스 | `UPTSkillBookWidget`, `UPTSkillListWidget`, `UPTSkillListEntryWidget` |

---

## 5. 전체 일정표

| Day | 날짜 | 요일 | 작업 | 산출물 |
|---|---|---|---|---|
| D1  | 05-20 | 수 | CommonUI 세팅, 프로젝트 모듈 구조 정리, 네이밍 컨벤션 적용 | 빌드 가능한 빈 UI 모듈 |
| D2  | 05-21 | 목 | `UPTUIManagerSubsystem` 골격 (위젯 생성/소멸, 레이어 enum) | 매니저 기본 API |
| D3  | 05-22 | 금 | 입력 모드 전환, 페이드 In/Out, `UPTUserWidgetBase` | 매니저 기능 완성 |
| D4  | 05-23 | 토 | 이벤트 델리게이트 정의 (`FPTOn...`), 시스템 측 브로드캐스트 지점 연결 | 이벤트 인프라 |
| D5  | 05-24 | 일 | `UPTHUDWidget` + `UPTPlayerStatusWidget` (HP/MP/Lv/Exp/Gold) | HUD 1차 |
| D6  | 05-25 | 월 | `UPTSkillSlotPanelWidget` + `UPTSkillSlotWidget` (쿨타임 포함) | HUD 완성 |
| D7  | 05-26 | 화 | `UPTInventoryWidget` + `UPTInventoryGridWidget` 그리드 레이아웃 | 인벤토리 골격 |
| D8  | 05-27 | 수 | `UPTInventorySlotWidget`, `UPTEquipmentSlotWidget` 장착 로직 | 슬롯/장착 |
| D9  | 05-28 | 목 | 드래그&드롭, 우클릭 사용/장착, `UPTItemTooltipWidget` | 인벤토리 완성 |
| D10 | 05-29 | 금 | `UPTSkillBookWidget` + `UPTSkillListWidget` / `UPTSkillListEntryWidget` | 스킬 창 1차 |
| D11 | 05-30 | 토 | 스킬 리스트 → 슬롯 배정 UX, MP/쿨타임 표시 | 스킬 창 완성 |
| D12 | 05-31 | 일 | `UPTMonsterHealthBarWidget` (WidgetComponent, 빌보드, 자동 숨김) | 일반몹 HP바 |
| D13 | 06-01 | 월 | `UPTBossHealthBarWidget` (페이즈 세그먼트, 등장/사망 연출) | 보스 HP바 |
| D14 | 06-02 | 화 | 임시 → 실제 아트 교체, 폰트/컬러 토큰화, WBP 정리 | 아트 연동 |
| D15 | 06-03 | 수 | 통합 QA, 입력 모드/포커스/토글 엣지 케이스 버그픽스 | MVP 완료 |

## 주차별 마일스톤

| 구간 | 기간 | 목표 |
|---|---|---|
| Week 1 (D1~D6) | 05-20 ~ 05-25 | 기반 인프라 + HUD 완료 |
| Week 2 (D7~D13) | 05-26 ~ 06-01 | 인벤토리 / 스킬 창 / 몹·보스 HP바 |
| Week 3 (D14~D15) | 06-02 ~ 06-03 | 아트 연동 + QA |
