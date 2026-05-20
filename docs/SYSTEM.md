# 통합 시스템 설계 개요

**프로젝트** : 악마 창궐 세계관 핵&슬래시 RPG
**문서 버전** : v1.1
**작성일** : 2026-05-20

> 각 파트별 세부 설계 문서의 요약 및 링크 모음. 시스템 간 연관 관계를 파악하기 위한 진입점 문서.

---

## 1. 플레이어 / 카메라 / 스킬 시스템

**담당** : 송승환
**세부 문서** : [[링크 삽입](https://www.notion.so/teamsparta/MVP-3652dc3ef514801d929df4f23e841d4d)]

플레이어 캐릭터의 스탯, 전투, 이동, 카메라, 스킬을 담당하는 핵심 시스템.
`ABaseCharacter`를 베이스로 플레이어·몬스터가 공통 전투 로직을 공유하며, 스탯 데이터는 `ABasePlayerState`에서 관리해 리스폰 후에도 유지된다.
모든 게임 로직 판정은 서버에서만 처리하고, 클라이언트는 입력 감지·연출·UI 갱신만 담당한다.

| 주요 구성 요소 | 설명 |
|---|---|
| `ABaseCharacter` | 플레이어·몬스터 공통 베이스. 전투 공식, ApplyDamage(), OnDeath() 담당 |
| `APlayerCharacter` | 입력 처리, 카메라, 컴포넌트 부착 |
| `ABasePlayerState` | 전투 스탯 복제 관리 (HP/MP/ATK/DEF 등) |
| `USkillComponent` | 스킬 슬롯, 쿨다운, MP 소모 처리 |
| 카메라 | 쿼터뷰 고정 (Pitch -55°), SpringArm + Camera |
| 네트워크 | Server RPC 기반 판정, PlayerState 복제 |

---

## 2. 몬스터 / NPC 시스템

**담당** : 변유민
**세부 문서 (몬스터)** : [[링크 삽입](https://www.notion.so/teamsparta/MVP-3652dc3ef51480c08c8dcf06c635fc95)]
**세부 문서 (NPC)** : [[링크 삽입](https://www.notion.so/teamsparta/NPC-MVP-3652dc3ef51480cba9f1e6b851520805)]

몬스터 AI, 전투 판정, 드롭, 스폰과 NPC 대화·퀘스트 수락을 담당하는 시스템.
Behavior Tree 기반으로 순찰 → 감지 → 추격 → 공격 흐름을 제어하며, 플레이어 파트와 `DT_Character` 단일 테이블을 공유한다.
MVP는 근접형 쫄몹 1종과 HP 50% 이하 시 패턴이 변하는 보스 1종, 퀘스트 NPC 1종을 구현한다.

| 주요 구성 요소 | 설명 |
|---|---|
| `AMonsterCharacter` | 몬스터 전용 로직 (AI, 드롭, 순찰) |
| `AMonsterAIController` | BT 실행 진입점 |
| `BT_Monster / BT_Boss` | 행동 트리 (쫄몹 / 보스 분리) |
| `DT_Character` | 플레이어·몬스터 통합 스탯 테이블 |
| 드롭 | 골드 / 경험치 / 장비 스폰 |
| `ANPCCharacter` | NPC 베이스 클래스. 대화 트리거 및 퀘스트 수락 처리 |
| 퀘스트 파티 공유 | 1인 수락 → Server RPC → 파티 전원 동시 등록 |

---

## 3. 아이템 / 장비 / 인벤토리 / 상호작용 시스템

**담당** : 고준혁
**세부 문서** : [[링크 삽입](https://www.notion.so/teamsparta/MVP-3652dc3ef514803db3fed809cee65774)]

아이템 데이터 구조, 인벤토리 관리, 장비 장착/해제, 오브젝트 상호작용을 담당하는 시스템.
장비 컴포넌트(`UEquipmentComponent`)와 인벤토리 컴포넌트(`UInventoryComponent`)를 `APlayerCharacter`에 부착하는 구조로, 장착 즉시 스탯이 PlayerState에 반영된다.

| 주요 구성 요소 | 설명 |
|---|---|
| `UInventoryComponent` | 아이템 가방 관리 (30칸, 5×6) |
| `UEquipmentComponent` | 장착 슬롯 관리 (무기/갑옷 2슬롯) + 스탯 합산 |
| 아이템 종류 | 장비 (무기/갑옷, Common/Rare), 소비 (물약) |
| 상호작용 | F키 (NPC/구조물), 좌클릭 (아이템 획득) |

---

## 4. UI 시스템

**담당** : 문인우
**세부 문서** : [[링크 삽입](https://www.notion.so/teamsparta/UI-MVP-3652dc3ef5148038ac50d8106bdd1f60)]

HUD, 인벤토리 창, 상점 창, 몹 체력바 등 모든 UI 화면을 담당하는 시스템.
UMG + CommonUI 기반으로, UI는 데이터를 직접 읽지 않고 각 시스템에서 발행하는 델리게이트 이벤트만 구독해 갱신한다.

| 주요 화면 | 설명 |
|---|---|
| HUD | HP/MP, 레벨/경험치, 스킬 슬롯 4개 + 쿨타임, 골드 |
| 인벤토리 / 장비창 | 5×6 그리드, 장착 슬롯 2개, 툴팁 |
| 일반몹 체력바 | 몹 액터에 부착, 피격 시 노출 |
| 보스 체력바 | 화면 상단, 페이즈 세그먼트 표시 |
| 스킬 창 | 스킬 목록, 슬롯 배치 |

---

## 5. 게임모드 / 서브시스템

**담당** : 정호승
**세부 문서** : [[링크 삽입](https://www.notion.so/teamsparta/MVP-3652dc3ef5148050b1c1fcafc3cfa6db)]

게임 전체 흐름 제어, 플레이어 스폰/리스폰, 승패 판정, 경제·진행·세이브 서브시스템을 담당하는 시스템.
`GameMode`는 서버 전용으로 판정 로직만 처리하고, 클라이언트에 알릴 때는 반드시 `GameState`를 경유한다.
챕터 이동이나 게임 재시작 후에도 유지되어야 하는 데이터(골드, 레벨, 경험치, 세이브)는 `UGameInstanceSubsystem` 기반으로 관리한다.

| 주요 구성 요소 | 설명 |
|---|---|
| `ABaseGameMode` | 서버 전용. 게임 상태 전환, 리스폰, 승패 판정, 경험치·드롭 분배 |
| `ABaseGameState` | 서버 + 클라이언트 복제. 상태 전파 및 UI 이벤트 전달 |
| `UEconomySubsystem` | 골드 보유량 관리, 수입/지출 처리 (개인별) |
| `UPlayerProgressSubsystem` | 레벨, 경험치 관리, 레벨업 이벤트 발행 |
| `USaveSubsystem` | 골드·레벨·인벤토리·장비 전체 저장·불러오기 |

**게임 상태 (EGamePhase)**

| 상태 | 전환 조건 |
|---|---|
| `Playing` | 기본 상태 |
| `BossFight` | 보스방 진입 |
| `GameOver` | 파티 전원 사망 (보스전에만 해당) |

---

## 시스템 간 주요 연관 관계

| 이벤트 / 데이터 | 발신 | 수신 |
|---|---|---|
| `OnHPChanged` / `OnMPChanged` | 플레이어 | UI |
| `OnPlayerDied` | 플레이어 | 게임모드 |
| `OnSkillCooldownStart/End` | 스킬 | UI |
| `OnEquipChanged` | 장비 (고준혁) | UI |
| `DT_Character` | 플레이어 | 몬스터 (공유) |
| `ApplyDamage()` | 몬스터 → 플레이어 | `ABaseCharacter` |
| 인벤토리 → 장비 장착 | 장비 (고준혁) | PlayerState 스탯 반영 |
| `OnGoldChanged` | `UEconomySubsystem` | UI 골드 표시 |
| `OnExpChanged` / `OnLevelUp` | `UPlayerProgressSubsystem` | UI, 플레이어 스탯 갱신 |
| `OnGamePhaseChanged` | `ABaseGameState` | UI, 각 시스템 |
| `SaveGame()` / `LoadGame()` | `USaveSubsystem` | 전체 서브시스템 |
| `OnNPCInteract` / `OnDialogueFinished` | NPC | UI (다이얼로그 위젯) |
| `AcceptQuest()` → `OnQuestAccepted` | NPC → 게임모드 | 파티 전원 퀘스트 등록 |
