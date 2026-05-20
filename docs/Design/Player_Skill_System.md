# 플레이어 / 카메라 / 스킬 시스템 설계서 — MVP

**프로젝트** : 악마 창궐 세계관 핵&슬래시 RPG
**문서 버전** : v0.4 (MVP)
**담당자** : 송승환
**작성일** : 2026-05-15
**엔진** : Unreal Engine 5.7 / C++ + Blueprint

---

## 0. MVP 범위 정의

| 항목 | MVP 포함 |
|------|----------|
| 플레이어 스탯 | HP / ATK / DEF / 이동속도 / 공격속도 / MP |
| 크리티컬 | 데미지 배율 200% 고정 |
| HP 재생 | 5초마다 Max HP 1% (항시, 서버 처리) |
| 사망 | OnPlayerDied 호출 + 경험치 차감 |
| 무기 | 근접 1종 |
| 스킬 | 일반 공격 (근접) |
| 장비 장착 | UEquipmentComponent, 무기/갑옷 2슬롯 |
| 카메라 | 쿼터뷰 고정 추적 |
| 네트워크 | PlayerState 복제, Server RPC 기반 판정 |

---

## 1. 플레이어 시스템 (Player System)

### 1.1 클래스 구조

```
ABaseCharacter (C++ 베이스 — 플레이어·몬스터 공통 상속)
 ├─ APlayerCharacter (C++ 파생)
 │    ├─ USpringArmComponent  (CameraBoom)
 │    ├─ UCameraComponent     (FollowCamera)
 │    ├─ UEquipmentComponent  (Equipment) — 고준혁
 │    ├─ USkillComponent      (Skill)
 │    └─ UInventoryComponent  (Inventory) — 고준혁
 └─ AMonsterCharacter (C++ 파생 — 변유민)

ABasePlayerState (APlayerState 파생 — 플레이어 전투 스탯 관리)
 ├─ 현재 HP / MaxHp
 ├─ 현재 MP / MaxMp
 ├─ BaseAtk (BaseAtk + 장비ATK + 버프)
 ├─ BaseDef (BaseDef + 장비DEF + 버프)
 ├─ AttackSpeed (기본값 + 장비보너스 + 버프)
 ├─ MoveSpeed (기본값 + 장비보너스 + 버프)
 ├─ CriticalChance (크리티컬 확률)
 └─ CriticalATK (크리티컬 데미지 배율)
```

- `ABaseCharacter`: `ApplyDamage()`, `OnDeath()` 등 공통 전투 로직
- `APlayerCharacter`: 입력 / 카메라 / 장비 컴포넌트 — **데이터는 PlayerState에서 읽어옴**
- `ABasePlayerState`: 플레이어 데이터 보관 + 복제 — **Pawn이 Destroy돼도 데이터 유지**
- `AMonsterCharacter`: AI / 드롭 / 순찰 등 몬스터 전용 로직 (변유민)

---

### 1.2 네트워크 구조 원칙

> **핵심 규칙**: 게임 로직 판정은 반드시 **서버에서만** 처리한다. 클라이언트는 입력 감지·시각 연출·UI 갱신만 담당한다.

| 주체 | 역할 |
|------|------|
| 서버 | 데미지 계산, HP 재생, 사망 판정, 스탯 변경 |
| 클라이언트 | 입력 감지 → Server RPC 호출, UI 갱신, 애니메이션 재생 |

`APlayerState`는 서버 + 모든 클라이언트에 복제되므로 파티원들도 서로의 데이터를 볼 수 있다.

PlayerState 스탯이 변경되면 `APlayerCharacter`가 값을 읽어 `CharacterMovement->MaxWalkSpeed` 및 `AnimInstance PlayRate`에 즉시 반영한다.

#### 입력 처리 흐름

```
클라이언트: 입력 감지 (Enhanced Input)
 → Server RPC 호출 (Server_Attack, Server_UseSkill 등)
      → 서버: 판정 및 데미지 계산
      → 서버: PlayerState 스탯 변경 → 복제 변수 갱신
           → 클라이언트: OnRep_HP() 호출 → UI 갱신
```

---

### 1.3 데이터 테이블 — DT_Character (몬스터 파트와 공유 단일 테이블)

플레이어와 몬스터를 하나의 테이블로 관리. 사용하지 않는 필드는 `0`으로 설정.

| 필드 | 플레이어 (1레벨) | 몬스터 (1레벨) | 비고 |
|------|:--------------:|:----------:|------|
| `MaxHp` | 100 | 50 | 최대 체력 |
| `BaseAtk` | 10 | 10 | 기본 공격력. 방어력보다 낮으면 최소 1 |
| `BaseDef` | 0 | 5 | 기본 방어력. 플레이어는 장비에서 합산 |
| `AttackSpeed` | 1.0 | 1.0 | AnimNotify 기반 |
| `MoveSpeed` | 600 | 500 | 언리얼 유닛 (cm/s) |
| `MP` | 100 | 0 | 몬스터 미사용 → 0 |
| `CriticalATK` | 200% | 0% | 몬스터 미사용 → 0 |
| `CriticalChance` | 0% | 0% | 장비 합산으로 증가 |
| `RequiredEXP` | 100 | 0 | 몬스터 미사용 → 0 |
| `SightAngle` | 0 | 360 | 플레이어 미사용 → 0 |
| `SightRange` | 0 | 1000 | 플레이어 미사용 → 0 |
| `GoldDropMin` | 0 | 500 | 플레이어 미사용 → 0 |
| `GoldDropMax` | 0 | 1000 | 플레이어 미사용 → 0 |
| `EquipDropRate` | 0 | 0.001 | 플레이어 미사용 → 0 |
| `RewardExp` | 0 | 10 | 플레이어 미사용 → 0 |

#### 레벨업 성장

- HP / ATK / MP 모두 **10% / 레벨** 증가
- 공식: `기본값 × (1.1 ^ (레벨 - 1))`

---

### 1.4 전투 공식

```
실제 데미지   = max((최종 ATK × 스킬배율) - 최종 DEF, 1)
크리티컬 적용 = 최종 ATK × CriticalATK
```

- DEF ≥ ATK 인 경우 **최소 데미지 1** 보장
- 데미지 계산은 **서버에서만** `ABaseCharacter::ApplyDamage()` 호출
- 클라이언트에서 직접 호출 금지 — 반드시 Server RPC 경유

---

### 1.5 공격 속도 & 애니메이션

- 공격 속도 1.0 = 애니메이션 PlayRate 1배
- 공속 변화 시 `AnimInstance`의 PlayRate를 배율에 맞게 스케일
- **AnimNotify** (`AN_AttackHit`) 로 히트박스 활성화 타이밍 결정
- MVP: 근접 무기 Montage 1종만 구현

---

### 1.6 HP 재생

- MVP 조건: 항시 재생 (전투 중 여부 분기는 MVP 이후)
- 주기: 5초마다 `MaxHp × 0.01` 회복
- 구현: `FTimerHandle` + `SetTimer()` — **서버에서만 실행**, 클라이언트 중복 실행 금지

---

### 1.7 사망 & 데스 페널티

1. HP ≤ 0 → `OnPlayerDied()` 호출
2. 경험치 일정량 차감 (수치 추후 확정)
3. 마을 내 세이브 포인트 상호작용 등록 → 사망 시 해당 오브젝트 앞 즉시 리스폰
4. 리스폰 처리는 **GameMode(정호승)에 위임**

---

## 2. 카메라 시스템 (Camera System)

### 2.1 개요

- **방식**: 쿼터뷰 (Pitch: -55°), 플레이어에 고정, 즉시 추적
- MVP에서 줌·회전 조작 없음

### 2.2 컴포넌트 구성

```
APlayerCharacter
 ├─ USpringArmComponent  (CameraBoom)
 └─ UCameraComponent     (FollowCamera)
```

### 2.3 SpringArm 설정값

| 파라미터 | 값 | 비고 |
|----------|----|------|
| `TargetArmLength` | 1500 cm | 줌 고정 |
| `SocketOffset` | (0, 0, 200) | 캐릭터 중심보다 약간 위 |
| `Rotation` | Pitch: -55°, Yaw: 0°, Roll: 0° | 쿼터뷰 고정 각도 |
| `bUsePawnControlRotation` | false | 마우스 회전 입력 무시 |
| `bInheritPitch / Yaw / Roll` | false | 고정 각도 유지 |
| `bEnableCameraLag` | false | 즉시 추적 |

### 2.4 Camera 설정값

| 파라미터 | 값 |
|----------|----|
| `bUsePawnControlRotation` | false |
| `ProjectionMode` | Perspective |
| `FOV` | 60° |

### 2.5 캐릭터 회전

- `bOrientRotationToMovement`: `true` — 이동 방향으로 자동 회전
- `bUseControllerRotationYaw`: `false` — 카메라와 캐릭터 Yaw 분리

---

## 3. 키 바인딩 (Key Binding)

**입력 시스템**: Enhanced Input 사용

| 키 | 동작 |
|----|------|
| 좌클릭 (지면) | 기본 공격 / 이동 |
| 좌클릭 (아이템 레이블) | 아이템 획득 (World UI 판정 분리) |
| 우클릭 | 이동 |
| Q / W / E / R | 스킬 슬롯 1~4 발동 |
| 1 / 2 / 3 / 4 | 소비 아이템 퀵슬롯 |
| F | NPC 대화 / 구조물 상호작용 |

---

## 4. 스킬 시스템 (Skill System)

### 4.1 개요

- 스킬 관련 로직을 `USkillComponent`로 분리하여 `APlayerCharacter`에 부착
- MVP: **일반 공격(근접) 1종**만 구현
- HUD 스킬 슬롯 **4개** 확정 (Q/W/E/R)

### 4.2 USkillComponent 구조

```
APlayerCharacter
 └─ USkillComponent
      ├─ 스킬 슬롯 관리 (슬롯 1~4)
      ├─ 쿨다운 관리 (FTimerHandle)
      ├─ MP 소모 처리 → PlayerState.CurrentMP 갱신
      └─ 스킬 발동 → AnimMontage 재생 → ApplyDamage() 호출
```

### 4.3 스킬 데이터 구조 (DT_Skill)

| 필드 | 타입 | 설명 |
|------|------|------|
| `SkillID` | FName | 고유 식별자 |
| `MPCost` | float | MP 소모량 |
| `Cooldown` | float | 쿨다운 (초) |
| `DamageMultiplier` | float | ATK 대비 데미지 배율 |
| `SkillType` | ESkillType | Active / Passive |
| `AnimMontage` | TSoftObjectPtr | 재생할 몽타주 |

### 4.4 일반 공격 (MVP)

- 근접 무기 Montage 재생 → `AN_AttackHit` AnimNotify → `ApplyDamage()` 호출
- MP 소모 없음 (일반 공격은 무료)
- 쿨다운: 공격 속도로 대체 (AnimMontage PlayRate 조절)

### 4.5 스킬 발동 흐름

```
클라이언트: 입력 감지 (Enhanced Input)
 → Server_UseSkill() RPC 호출
      → 서버: USkillComponent.TryActivateSkill(SkillID)
              MP 충분?
              Yes → PlayerState.CurrentMP 차감 → OnRep_MP() → UI 갱신
                  → AnimMontage 재생 (Multicast)
                  → AN_SkillHit (AnimNotify) → ApplyDamage() 서버 처리
                  → FTimerHandle 쿨다운 시작
                  → OnSkillCooldownStart 발행
              No  → Client RPC → UI 피드백 (MP 부족)
```

---

## 5. 파트 작업 분해 (MVP)

| # | 작업 | 선행 | 산출물 | 예상 시간 | 일차 |
|---|------|------|--------|-----------|------|
| 1 | `DT_Character` 구조 확정 (몬스터 파트 협의) | — | DataTable 에셋 | 7h | 1일차 |
| 2 | `ABaseCharacter` 베이스 클래스 구현 | #1 | ABaseCharacter | 6h | 3일차 |
| 3 | `ABasePlayerState` 구현 (스탯 복제 변수) | #1 | ABasePlayerState | 4h | 2일차 |
| 4 | `APlayerCharacter` 파생 클래스 구현 | #2 | APlayerCharacter | 6h | 5일차 |
| 5 | 카메라 컴포넌트 설정 (SpringArm + Camera) | #4 | 카메라 셋업 | 3h | 6일차 |
| 6 | 전투 공식 + `ApplyDamage()` (서버 전용) | #2 | ABaseCharacter | 7h | 4일차 |
| 7 | Server RPC 구현 (Server_Attack, Server_UseSkill) | #4 | Server RPC | 7h | 6~7일차 |
| 8 | HP 재생 타이머 (서버 전용) | #3 | FTimerHandle | 4h | 3~4일차 |
| 9 | 사망 처리 + `OnPlayerDied` 발행 | #6 | OnDeath 로직 | 5h | 5~6일차 |
| 10 | `DT_Skill` 구조 확정 | #1 | DataTable 에셋 | 4h | 2일차 |
| 11 | `USkillComponent` 구현 (슬롯 / 쿨다운 / MP 소모) | #4 #10 | USkillComponent | 7h | 7~8일차 |
| 12 | 일반 공격 Montage + AnimNotify 연동 | #11 | AN_AttackHit | 6h | 8~9일차 |
| 13 | 스킬 발동 흐름 + Server RPC 연동 | #12 | DT_Skill 연동 | 7h | 9~10일차 |
| 14 | UI 이벤트 델리게이트 연동 | #3 #8 #9 #13 + UI 협의 | 델리게이트 바인딩 | 8h | 10~11일차 |

> **총 예상 시간**: 약 81h (하루 8시간 기준 약 11일)
