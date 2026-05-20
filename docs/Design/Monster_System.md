# 몬스터 시스템 설계서 — MVP

**프로젝트** : 악마 창궐 세계관 핵&슬래시 RPG
**문서 버전** : v1.6 (MVP, 요약본)
**담당자** : 변유민
**작성일** : 2026-05-15
**엔진** : Unreal Engine 5.6 / C++ + Blueprint

---

## 1. 개요

핵&슬래시 RPG의 몬스터 시스템 MVP 설계 요약본.
다른 시스템과는 이벤트/인터페이스 기반의 느슨한 결합을 유지한다.

### 1.1 몬스터 시스템 책임 범위

- 몬스터 스탯(DT) 정의 및 로드 — `DT_Character` (플레이어 / 몬스터 / 보스 통합)
- Behavior Tree 기반 AI 행동 제어 — 순찰 → 감지 → 추격 → 공격
- 피해 계산 및 사망 처리
- 드롭 아이템 / 경험치 / 골드 스폰

### 1.2 구현 방식

- C++ 베이스 클래스(`ABaseCharacter`) + Blueprint 파생 구조
- Behavior Tree + Blackboard (언리얼 AI 시스템)
- 게임 로직과 데이터 분리 — 스탯은 DataTable에서 로드
- 타 시스템과 델리게이트(Delegate) 단방향 이벤트 통신

### 1.3 ABaseCharacter 공통 구조

플레이어/몬스터 공통 베이스 클래스. 스탯 로드, 피해 수신, 사망 판정을 담당한다.
파생 클래스(플레이어/몬스터)에서 부활, 드롭 등 고유 동작을 오버라이드한다.

> 향후 HealthComponent / CombatComponent / DropComponent 분리 예정

---

## 2. MVP 몬스터 목록

| 종류 | 등급 | 행동 패턴 | MVP 구현 |
|------|------|-----------|----------|
| 던전 쫄몹 | Normal | 근접형 | ✅ 1종 |
| 챕터 보스 | Boss | 근접형 + 특수 패턴 (HP 50% 이하 시 강력한 공격) | ✅ 1종 |

---

## 3. 스탯 데이터 테이블 (DT) 정의

### 3.1 통합 DataTable — `DT_Character`

플레이어 / 쫄몹 / 보스를 하나의 DataTable로 관리한다.
사용하지 않는 필드는 `0`으로 설정한다.

| 필드 | 플레이어 (1레벨) | 쫄몹 (1레벨) | 보스 | 설명 |
|------|:--------------:|:----------:|:----:|------|
| `MaxHp` | 100 | 50 | 기획 확정 | 최대 체력. 인스턴스 초기화 시 `Hp = MaxHp` |
| `BaseAtk` | 10 | 10 | 기획 확정 | 기본 공격력. 방어력보다 낮으면 최소 1 |
| `BaseDef` | 0 | 5 | 기획 확정 | 기본 방어력. 플레이어는 장비에서 합산 |
| `AttackSpeed` | 1.0 | 1.0 | 1.0 | AnimNotify 기반. 애니메이션 재생 속도 연동 |
| `MoveSpeed` | 600 | 500 | 기획 확정 | 언리얼 유닛 |
| `SightAngle` | 0 | 140 | 180 | 감지 각도 (도 단위). 플레이어 미사용 → 0 |
| `SightRange` | 0 | 800 | 1000 | 플레이어 감지 거리 (유닛). 플레이어 미사용 → 0 |
| `ChaseRange` | 0 | 1200 | 1500 | 추격 유지 거리 (유닛). 초과 시 Chase 해제. 플레이어 미사용 → 0 |
| `AttackRange` | 0 | 150 | 300 | 공격 판정 거리 (유닛). 플레이어 미사용 → 0 |
| `PatrolRadius` | 0 | 1000 | 1000 | 순찰 반경 (유닛). 스폰 위치 기준. 플레이어 미사용 → 0 |
| `MaxChaseDistance` | 0 | 1000 | 1000 | 스폰 위치 기준 최대 이탈 거리. 초과 시 강제 Patrol 복귀. 플레이어 미사용 → 0 |
| `GoldDropMin` | 0 | 500 | 기획 확정 | 골드 드롭 최솟값. 플레이어 미사용 → 0 |
| `GoldDropMax` | 0 | 1000 | 기획 확정 | 골드 드롭 최댓값. 플레이어 미사용 → 0 |
| `EquipDropRate` | 0 | 0.001 | 0.001 | 장비 드롭 확률 (0.0 ~ 1.0). 플레이어 미사용 → 0 |
| `RewardExp` | 0 | 10 | 기획 확정 | 처치 시 지급 경험치. 플레이어 미사용 → 0 |

> 플레이어 고유 필드 (Critical, MP, DeathPenalty 등)는 플레이어 파트(송승환)가 별도 관리.
> 보스 수치는 기획 확정 후 입력한다.

### 3.2 레벨 스케일링

레벨업 시 스탯이 기본값의 10%씩 증가한다. 챕터(지역)에 따라 배치 레벨 구간을 설정한다.

| 챕터 | 몬스터 레벨 | 비고 |
|------|------------|------|
| 1챕터 (광산) | 1 ~ 5 | 광부 반란군, 언데드 |

---

## 4. 몬스터 상태 (State)

몬스터는 항상 아래 5가지 상태 중 하나를 유지한다.
상태 전환과 행동은 **BT가 전담**하며, State는 현재 상태를 나타내는 읽기 전용 값으로만 사용한다.

### 4.1 상태 정의

| 상태 | 설명 | 전환 조건 |
|------|------|-----------|
| `Idle` | 배치된 자리에서 대기 | 초기 상태. 3초 경과 후 Patrol 전환 |
| `Patrol` | 스폰 위치 기준 반경 1000 유닛 내 랜덤 이동 | Idle 3초 경과 또는 Chase 해제 후 3초 경과 시 |
| `Chase` | 감지된 플레이어 추격 | 플레이어 감지 시 |
| `Attack` | 공격 범위 내 진입 시 공격 | 플레이어가 공격 범위 이내 |
| `Dead` | 사망 처리 | HP 0 이하 |

Chase 탈출 조건: 플레이어와의 거리 ChaseRange 초과 시 → Patrol 복귀 / 스폰 위치로부터 MaxChaseDistance 초과 시 → 강제 Patrol 복귀

### 4.2 상태 전환 흐름

```
Idle → 3초 경과 → Patrol
Patrol → 플레이어 SightRange 이내 감지 → Chase
Chase → 공격 범위 이내 → Attack
      → ChaseRange 초과 또는 MaxChaseDistance 초과 → 3초 후 Patrol 복귀
Attack → 범위 이탈 → Chase 복귀
       → HP 0 → Dead
```

> State 값은 BT Task 내에서 갱신되며, 외부 참조용(애니메이션 블렌딩, UI)으로만 사용한다.

---

## 5. AI 아키텍처

### 5.1 구성

- **AMonsterAIController** — 몬스터 전용 AIController, BT 실행 진입점
- **BT_Monster** — 기본 쫄몹 Behavior Tree
- **BT_Boss** — 보스 전용 Behavior Tree
- **BB_Monster** — 공유 Blackboard
- **게임 시스템 ↔ 몬스터 AI** : AIPerception 이벤트 + 델리게이트 단방향 통신

### 5.2 Behavior Tree 흐름

```
공격 가능 조건 충족 시 → 공격 실행 → 애니메이션 완료 후 재공격 대기 또는 추격 복귀
플레이어 감지 시 → 추격
감지 없을 시 → 스폰 위치 기준 반경 내 랜덤 순찰
```

> 공격 애니메이션 시작 후 플레이어 이탈 시 현재 공격 완료 후 Chase 전환. 도중 강제 중단 없음.

### 5.3 Blackboard Key 정의

| Key 이름 | 타입 | 설명 |
|----------|------|------|
| `TargetActor` | Object | 감지된 플레이어 레퍼런스 |
| `IsTargetDetected` | Bool | SightRange 이내 플레이어 감지 여부 |
| `IsInAttackRange` | Bool | 플레이어가 공격 범위 이내 여부 |
| `CanAttack` | Bool | 공격 가능 여부. 애니메이션 완료 후 true 복귀 |
| `IsEnraged` | Bool | 보스 전용. HP 50% 이하 시 true. 강력한 공격으로 전환 트리거 |
| `PatrolLocation` | Vector | 순찰 목표 좌표. 스폰 위치 기준 반경 내 랜덤 |
| `SpawnLocation` | Vector | 최초 배치 좌표. 순찰 / 추격 해제 기준점 |

### 5.4 감지 설정 (AIPerception)

| 항목 | 쫄몹 | 보스 |
|------|------|------|
| 감지 타입 | Sight (시각) | Sight (시각) |
| 시야 각도 | 140° | 180° |
| 감지 거리 (SightRange) | 800 유닛 | 1000 유닛 |
| 추격 유지 거리 (ChaseRange) | 1200 유닛 | 1500 유닛 |
| 맵 이탈 방지 거리 (MaxChaseDistance) | 스폰 위치 기준 1000 유닛 | 스폰 위치 기준 1000 유닛 |
| 감지 갱신 | 이벤트 기반 | 이벤트 기반 |

### 5.5 행동 패턴 유형 (MVP)

| 유형 | 대상 | 설명 |
|------|------|------|
| 근접형 | 쫄몹 1종 | 감지 즉시 추격 → 공격 범위(150) 진입 시 근접 공격 |
| 근접형 + 특수 패턴 | 보스 1종 | 공격 범위(300) 근접 공격. HP 50% 이하 시 강력한 공격으로 전환 |

---

## 6. 스폰 시스템

| 항목 | 내용 |
|------|------|
| 방식 | 에디터 직접 배치 (Place Actor) |
| 관리 | 게임모드가 스폰 관리 없이 배치된 액터를 그대로 사용 |
| 재생성 | MVP에서는 리스폰 없음 — 처치 후 Destroy로 제거 |

---

## 7. 전투 메커니즘

### 7.1 피해 계산

공격자가 기본 피해값을 전달하고, 피격자가 방어력을 적용해 최종 피해를 계산한다. 최소 피해는 1 보장.

### 7.2 공격 판정 구조

공격 모션 시작 → 특정 타이밍에 범위 판정 → 범위 내 플레이어에게 피해 적용.
한 번 피해를 입은 대상은 동일 공격에서 중복 피해 없음.

| 항목 | 값 |
|------|----|
| 판정 방식 | SphereTrace |
| 판정 타이밍 | AnimNotify |
| 판정 반경 | 쫄몹 150 / 보스 300 |
| 중복 히트 방지 | 피격 액터 등록 방식 |

### 7.3 피격 시스템

| 항목 | 내용 |
|------|------|
| 피격 애니메이션 | HitReact 몽타주 재생 (공격 중엔 스킵 가능) |
| 무적 시간 | MVP에서는 미적용 |
| 넉백 | MVP에서는 미적용 |

### 7.4 사망 처리 흐름

```
HP 0 이하 시
  → 이동 / 콜리전 / AI 즉시 차단
  → 사망 애니메이션 재생
  → 드롭 스폰
  → 제거
```

---

## 8. MVP 범위 정의

### 8.1 MVP 구현 목록 ✅

| 항목 | 내용 |
|------|------|
| 몬스터 | 쫄몹 1종 / 보스 1종 |
| AI | 상태 머신 + Behavior Tree |
| 전투 | 공격 판정 / 피격 처리 / 사망 처리 |
| 드롭 | 골드 / 경험치 / 장비 |
| 연동 | UI 이벤트 발행 |

### 8.2 MVP 이후 확장 항목 🔜

원거리형 몬스터, 다중 보스 패턴, 엘리트 몬스터 등

---

## 9. 파트 작업 분해 (MVP)

| # | 작업 | 선행 | 산출물 | 상태 |
|---|------|------|--------|------|
| 1 | `DT_Character` 구조 확정 | — | DataTable 에셋 | |
| 2 | `ABaseCharacter` C++ 구현 | #1 | ABaseCharacter | |
| 3 | `AMonsterCharacter` C++ 파생 클래스 구현 | #2 | AMonsterCharacter | |
| 4 | 몬스터 상태(`EMonsterState`) 정의 | #3 | EMonsterState | |
| 5 | `AMonsterAIController` + AIPerception 설정 | #3 | AMonsterAIController | |
| 6 | Blackboard(`BB_Monster`) Key 정의 | #4 + #5 | BB_Monster | |
| 7 | `BT_Monster` 기본 흐름 구현 | #6 | BT_Monster | |
| 8 | BTTask 구현 (MoveToTarget / Attack / Patrol) | #7 | BTTask 클래스들 | |
| 9 | 공격 판정 구현 (AnimNotify + SphereTrace + TSet) | #8 | HitCheck 로직 | |
| 10 | 피격 시스템 구현 (TakeDamage → 이벤트 발행) | #2 | TakeDamage | |
| 11 | 드롭 시스템 구현 (골드 / 경험치 / 장비) | #3 | DropComponent | |
| 12 | 쫄몹 1종 Blueprint 제작 (`BP_Monster_Normal`) | #8 + #11 | BP_Monster_Normal | |
| 13 | `BT_Boss` 구현 (HP 50% 체크 BTService + `IsEnraged`) | #7 | BT_Boss | |
| 14 | 보스 1종 Blueprint 제작 (`BP_Monster_Boss`) | #13 + #11 | BP_Monster_Boss | |
| 15 | UI 이벤트 델리게이트 연동 | #14 | 델리게이트 바인딩 | |
| 16 | 레벨 배치 및 스폰 테스트 | #12 + #14 | 테스트 맵 | |

---

## 10. 몬스터 + NPC 통합 15일 일정표

| Day | 작업 | 구분 | 예상 시간 |
|-----|------|------|----------|
| Day 1 | M#1 DT_Character 구조 확정 | 몬스터 | 6h |
| Day 2 | M#2 ABaseCharacter C++ 구현 | 몬스터 | 6h |
| Day 3 | M#3 AMonsterCharacter 구현 + M#4 EMonsterState 정의 | 몬스터 | 7h |
| Day 4 | M#5 AIController + AIPerception 설정 | 몬스터 ⚠️ | 7~8h |
| Day 5 | M#6 Blackboard Key 정의 + N#1 ANPCCharacter 구현 | 병렬 | 7h |
| Day 6 | M#7 BT_Monster 기본 흐름 구현 | 몬스터 ⚠️ | 7~8h |
| Day 7 | M#8 BTTask 구현 3종 (MoveToTarget / Attack / Patrol) | 몬스터 ⚠️ | 7~8h |
| Day 8 | M#9 공격 판정 구현 + N#2 ENPCState 정의 | 병렬 | 7h |
| Day 9 | M#10 피격 시스템 + N#3 인터랙션 연동 (F키·200유닛) | 병렬 | 7h |
| Day 10 | M#11 드롭 시스템 + N#4 다이얼로그 이벤트 연동 | 병렬 | 7h |
| Day 11 | M#12 쫄몹 Blueprint 제작 + N#5 퀘스트 수락 Server RPC | 병렬 | 7h |
| Day 12 | M#13 BT_Boss + IsEnraged 구현 + N#6 파티 Multicast 전파 | 병렬 ⚠️ | 7~8h |
| Day 13 | M#14 보스 Blueprint 제작 + N#7 퀘스트 자동 완료 연동 | 병렬 | 7h |
| Day 14 | M#15 UI 델리게이트 연동 + N#8 NPC Blueprint 제작 | 병렬 | 7h |
| Day 15 | M#16 레벨 배치 + 스폰 테스트 + 전체 버그 수정 | 마무리 | 6~7h |

### ⚠️ 주의 구간

- **Day 4** — AIPerception 설정값 많고 디버깅 어려움
- **Day 6~7** — BT 개념 파악 + BTTask 작성법이 초보에게 가장 어려운 구간
- **Day 12** — BT_Boss(BTService) + Multicast RPC 동시 진행

---

## 11. 변경 이력

| 버전 | 날짜 | 내용 |
|------|------|------|
| v0.1 | 2026-05-15 | 초안 작성 |
| v0.9 | 2026-05-15 | MVP 정제 |
| v1.0 | 2026-05-15 | ABaseCharacter 구조, 몬스터 State, 전투/피격 시스템, 스폰 시스템 추가 |
| v1.2 | 2026-05-15 | State·BT 역할 분리, 무한 공격 방지, TSet 초기화 보장 |
| v1.3 | 2026-05-15 | 코드 블록 추상화, 섹션 정리 |
| v1.5 | 2026-05-15 | 시야각 확정, Idle 3초, 보스 특수 패턴 및 IsEnraged 추가 |
| v1.6 | 2026-05-15 | SightRange/ChaseRange 분리, MaxChaseDistance 추가 |
