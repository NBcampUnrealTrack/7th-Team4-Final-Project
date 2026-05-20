# 코딩 컨벤션 (Coding Convention)

---

## 목차

1. [C++ 네이밍 규칙](#1-c-네이밍-규칙)
2. [에셋 네이밍 규칙](#2-에셋-네이밍-규칙)
3. [코드 포맷](#3-코드-포맷)
4. [현대 C++ 문법](#4-현대-c-문법)
5. [헤더 파일 관리](#5-헤더-파일-관리)
6. [주석 작성법](#6-주석-작성법)
7. [Blueprint 연동 규칙](#7-blueprint-연동-규칙)
8. [폴더 구조](#8-폴더-구조)
9. [클래스 설계 원칙](#9-클래스-설계-원칙)

---

## 1. C++ 네이밍 규칙

### 1-1. 타입 접두사

| 접두사 | 대상 | 예시 |
|--------|------|------|
| `A` | `AActor` 상속 클래스 | `APlayerCharacter`, `AMonsterBase` |
| `U` | `UObject` 상속 클래스 (컴포넌트 등) | `UHealthComponent`, `USkillComponent` |
| `F` | 일반 구조체 및 일반 클래스 | `FDamageInfo`, `FCharacterStats` |
| `S` | Slate 위젯 클래스 | `SInventoryPanel` |
| `I` | 추상 인터페이스 클래스 | `IDamageable`, `IInteractable` |
| `E` | 열거형(Enum) | `EAttackType`, `EBossPhase` |
| `T` | 템플릿 클래스 | `TArray`, `TMap`, `TObjectPtr` |
| `b` | bool 변수 (소문자 접두사) | `bIsDead`, `bCanAttack`, `bIsGrounded` |

---

### 1-2. 일반 네이밍 규칙

- 모든 이름은 **PascalCase** 사용 (단어 첫 글자 대문자, 언더스코어 미사용)
- 모든 식별자(변수, 함수, 클래스, 에셋, 폴더)는 **영어**로 작성
- **변수·클래스 이름은 명사**, **함수 이름은 동사**
- 과도한 약어 사용 금지
- `Handle`, `Process` 같은 의미 없는 동사 접두사 사용 금지

```cpp
// ✅
float MaxHealth;
int32 CurrentComboCount;
bool bIsStunned;
void TakeDamage(float Amount);
bool CanAttack() const;

// ❌
float maxHP;            // PascalCase 아님, 축약어
bool dead;              // b 접두사 없음
void HandleDamage();    // 의미 없는 Handle 동사
void ProcessAtk();      // 축약어
```

---

### 1-3. 함수 네이밍

| 함수 유형 | 규칙 | 예시 |
|-----------|------|------|
| bool 반환 함수 | 의문형으로 작성 | `IsAlive()`, `CanUseSkill()`, `ShouldRespawn()` |
| 이벤트 핸들러 | `On` 으로 시작 | `OnDeath()`, `OnSkillActivated()` |
| RepNotify 함수 | `OnRep_` 접두사 | `OnRep_Health`, `OnRep_SkillState` |
| RPC 함수 | Server / Client / Multicast 접두사 | `ServerFireWeapon()`, `ClientNotifyDeath()` |
| Getter | `Get` 접두사 | `GetCurrentHealth()`, `GetAttackDamage()` |
| Setter | `Set` 접두사 | `SetMaxHealth(float Value)` |

---

### 1-4. 파라미터 네이밍

- 참조로 값을 출력하는 파라미터: `Out` 접두사
- bool 출력 파라미터: `bOut` 접두사
- 템플릿 파라미터: `In` 접두사

```cpp
void GetCharacterStats(FCharacterStats& OutStats);
bool TryGetTarget(AActor*& OutTarget);

template <typename InElementType>
class TMyContainer { ... };
```

---

### 1-5. 포터블 기본 타입

| 사용 (UE 타입) | 사용 금지 |
|----------------|-----------|
| `int32` | `int` |
| `uint8` | `unsigned char` |
| `uint16` / `uint32` / `uint64` | `unsigned short` / `unsigned int` / `unsigned long long` |
| `bool` | `BOOL` |
| `FString` | `std::string` |
| `TCHAR` | `char` |
| `PTRINT` | `intptr_t` |

---

## 2. 에셋 네이밍 규칙

### 2-1. 기본 패턴

```
[타입접두사]_[에셋기본이름]_[설명자]_[선택적변형]
```

예: `T_Rock_D`, `BP_EnemySword_Elite`, `AM_Player_Attack_Combo1`

---

### 2-2. 에셋 타입별 접두사

#### Blueprint / 스크립트

| 에셋 타입 | 접두사 | 예시 |
|-----------|--------|------|
| Blueprint Class | `BP_` | `BP_PlayerCharacter`, `BP_BossSlime` |
| Blueprint Interface | `BPI_` | `BPI_Damageable`, `BPI_Interactable` |
| Widget Blueprint (UMG) | `WBP_` | `WBP_HUD`, `WBP_SkillSlot` |
| Animation Blueprint | `ABP_` | `ABP_Player`, `ABP_MonsterBoss` |
| AI Controller (BP) | `AIC_` | `AIC_MonsterBase` |
| Behavior Tree | `BT_` | `BT_MonsterChase` |
| Blackboard | `BB_` | `BB_Monster` |
| GameplayAbility (GAS) | `GA_` | `GA_Slash`, `GA_Dodge` |
| GameplayEffect (GAS) | `GE_` | `GE_DamagePhysical`, `GE_HealPotion` |

#### 메시 / 텍스처

| 에셋 타입 | 접두사 | 예시 |
|-----------|--------|------|
| Static Mesh | `SM_` | `SM_Sword_Iron`, `SM_Rock_Large` |
| Skeletal Mesh | `SK_` | `SK_Player`, `SK_MonsterOrc` |
| Texture | `T_` | `T_Player_D`, `T_Rock_N` |
| Material | `M_` | `M_Sword_Iron`, `M_Ground_Dirt` |
| Material Instance | `MI_` | `MI_Sword_Gold`, `MI_Ground_Snow` |
| Material Function | `MF_` | `MF_BlendTwoTextures` |
| Physics Asset | `PHYS_` | `PHYS_Player` |
| Physical Material | `PM_` | `PM_Metal`, `PM_Stone` |

#### 애니메이션

| 에셋 타입 | 접두사 | 예시 |
|-----------|--------|------|
| Animation Sequence | `A_` | `A_Player_Run`, `A_Player_Attack1` |
| Animation Montage | `AM_` | `AM_Player_SkillSlash`, `AM_Boss_Roar` |
| Blend Space | `BS_` | `BS_Player_Locomotion` |
| Aim Offset | `AO_` | `AO_Player_Aim` |

#### 오디오 / 이펙트 / 기타

| 에셋 타입 | 접두사 | 예시 |
|-----------|--------|------|
| Sound Wave | `SW_` | `SW_Sword_Swing` |
| Sound Cue | `SC_` | `SC_Player_FootStep` |
| Niagara System | `NS_` | `NS_BloodSplash`, `NS_SkillFire` |
| Data Table | `DT_` | `DT_MonsterStats`, `DT_ItemData` |
| Data Asset | `DA_` | `DA_SkillConfig_Slash` |
| Curve | `Curve_` | `Curve_DamageRamp` |
| Enumeration | `E` | `EAttackType` (언더스코어 없음) |
| Structure | `F` | `FMonsterData` (언더스코어 없음) |

---

### 2-3. 텍스처 접미사

| 접미사 | 용도 |
|--------|------|
| `_D` | Diffuse / Albedo / Base Color |
| `_N` | Normal Map |
| `_R` | Roughness |
| `_M` | Metallic |
| `_AO` | Ambient Occlusion |
| `_E` | Emissive |
| `_A` | Alpha / Opacity Mask |

예: `T_Armor_Knight_D`, `T_Armor_Knight_N`, `T_Armor_Knight_R`

---

## 3. 코드 포맷

### 3-1. 중괄호

중괄호는 항상 **새 줄**에 작성한다. 한 줄 블록도 중괄호를 생략하지 않는다.

```cpp
// ✅
if (bIsDead)
{
    Destroy();
}

// ❌
if (bIsDead) { Destroy(); }
if (bIsDead)
    Destroy();
```

---

### 3-2. 들여쓰기

- **스페이스(Space)** 사용, 탭 사용 금지
- 들여쓰기 크기: 4칸

---

### 3-3. 포인터 및 참조 선언

포인터(`*`)와 참조(`&`) 기호는 **타입 쪽**에 붙인다.

```cpp
// ✅
AActor* MyActor;
const FVector& Location;

// ❌
AActor *MyActor;
AActor * MyActor;
```

---

### 3-4. Switch 문

모든 case에 `break` 또는 `// falls through` 주석을 명시하고, `default`는 항상 포함한다.

```cpp
switch (AttackType)
{
    case EAttackType::Slash:
        PerformSlash();
        break;

    case EAttackType::Thrust:
        // falls through

    case EAttackType::Heavy:
        PerformHeavyAttack();
        break;

    default:
        break;
}
```

---

### 3-5. If-Else

모든 실행 블록에 중괄호를 사용한다.

```cpp
if (Health > 75.f)
{
    SetState(ECharacterState::Healthy);
}
else if (Health > 25.f)
{
    SetState(ECharacterState::Injured);
}
else
{
    SetState(ECharacterState::Critical);
}
```

---

### 3-6. 기타

- `.cpp` / `.h` 파일 마지막 줄은 **빈 줄**로 끝낸다
- 함수 이름과 괄호 사이 **공백 없음**: `Attack()` (O), `Attack ()` (X)
- 변수는 **한 줄에 하나씩** 선언
- 이름 없는 리터럴 대신 **이름 있는 상수** 사용

```cpp
// ✅
const float KnockbackForce = 500.f;
LaunchCharacter(FVector::UpVector * KnockbackForce, true, true);

// ❌
LaunchCharacter(FVector::UpVector * 500.f, true, true);
```

- 복잡한 조건식은 **중간 변수**로 분리

```cpp
// ✅
const bool bIsAliveAndGrounded = IsAlive() && IsGrounded();
const bool bCanCounter = bIsAliveAndGrounded && bIsInCounterWindow;
if (bCanCounter)
{
    PerformCounter();
}
```

---

## 4. 현대 C++ 문법

### 4-1. nullptr

`NULL` 또는 `0` 대신 `nullptr`를 사용한다.

```cpp
AActor* Target = nullptr;   // ✅
AActor* Target = NULL;      // ❌
```

---

### 4-2. override / final

가상 함수 오버라이드 시 `virtual`과 `override`를 **반드시 함께** 명시한다.

```cpp
// ✅
virtual void BeginPlay() override;
virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                         AController* EventInstigator, AActor* DamageCauser) override;
```

파생이 설계되지 않은 클래스에는 `final`을 사용한다.

```cpp
class UBossHealthComponent final : public UHealthComponent { ... };
```

---

### 4-3. auto — 제한적 허용

아래 세 가지 경우에만 `auto` 사용을 허용한다.

```cpp
// ✅ 허용: 람다 변수
auto OnKilled = [this](AActor* Killed) { AddScore(Killed); };

// ✅ 허용: 장황한 반복자
for (auto It = MyMap.CreateIterator(); It; ++It) { ... }

// ✅ 허용: 템플릿에서 타입을 특정할 수 없는 경우
template <typename T>
auto GetValue(T& Container) -> decltype(Container.Get());

// ❌ 금지
auto Value = GetCurrentHealth();
```

`auto` 사용 시 `const`, `&`, `*` 한정자를 명시한다.

```cpp
const auto* Mesh = GetMesh();
auto& StatsRef = GetStats();
```

---

### 4-4. 범위 기반 for 루프

```cpp
for (AActor* Enemy : EnemyList)
{
    Enemy->TakeDamage(10.f);
}

for (TPair<FString, int32>& Kvp : SkillLevelMap)
{
    UE_LOG(LogTemp, Log, TEXT("Skill: %s, Level: %d"), *Kvp.Key, Kvp.Value);
}
```

---

### 4-5. 람다 캡처 — 명시적 캡처 필수

자동 캡처(`[&]`, `[=]`)는 **사용 금지**. 필요한 변수를 명시적으로 캡처한다.

```cpp
// ✅
float Damage = CalculateDamage();
GetWorldTimerManager().SetTimer(TimerHandle,
    [this, Damage]()
    {
        ApplyDamage(Damage);
    },
    0.5f, false);

// ❌
GetWorldTimerManager().SetTimer(TimerHandle,
    [&]() { ApplyDamage(Damage); },
    0.5f, false);
```

지연 실행 람다에서 UObject 캡처 시 `CreateWeakLambda` 또는 `TWeakObjectPtr`을 사용한다.

```cpp
// ✅
TWeakObjectPtr<AMyCharacter> WeakSelf = this;
FTimerDelegate Delegate = FTimerDelegate::CreateWeakLambda(this,
    [WeakSelf]()
    {
        if (WeakSelf.IsValid())
        {
            WeakSelf->DoSomething();
        }
    });
```

---

### 4-6. enum class

`enum class`를 사용한다. Blueprint에 노출되는 열거형은 기반 타입을 `uint8`로 지정한다.

```cpp
// ✅
UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Slash   UMETA(DisplayName = "Slash"),
    Thrust  UMETA(DisplayName = "Thrust"),
    Ranged  UMETA(DisplayName = "Ranged"),
};
```

플래그 열거형은 `ENUM_CLASS_FLAGS`를 사용하고, 비교 시 `!= None`을 명시한다.

```cpp
UENUM(BlueprintType, Meta = (Bitflags))
enum class EStatusEffect : uint8
{
    None   = 0x00,
    Stun   = 0x01,
    Burn   = 0x02,
    Freeze = 0x04,
};
ENUM_CLASS_FLAGS(EStatusEffect)

if ((CurrentEffects & EStatusEffect::Stun) != EStatusEffect::None) { ... }
```

---

### 4-7. Const 정확성

- 수정하지 않는 파라미터: `const` 선언
- 멤버 상태를 바꾸지 않는 함수: `const` 명시
- 반환 타입에는 `const` 붙이지 않음

```cpp
// ✅
bool IsAlive() const;
void PrintStats(const FCharacterStats& Stats) const;
void SetTargetList(TArray<AActor*> InTargets) { Targets = MoveTemp(InTargets); }

// ❌
const TArray<AActor*> GetTargets();
```

---

### 4-8. 기본 멤버 초기화

```cpp
UCLASS()
class UCharacterStats : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Stats")
    float MaxHealth = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Stats")
    int32 Level = 1;
};
```

---

### 4-9. 이동 시맨틱

컨테이너에 값을 전달할 때 `MoveTemp()`를 사용한다.

```cpp
void FInventory::SetItems(TArray<FItemData> InItems)
{
    Items = MoveTemp(InItems);
}
```

---

## 5. 헤더 파일 관리

### 5-1. 헤더 가드

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"
```

---

### 5-2. Include 최소화

- 헤더에서 포인터·참조로만 사용하는 타입은 **전방 선언** 사용
- 필요한 헤더는 **직접** include (간접 include 의존 금지)
- `Core.h` 같은 통합 헤더 대신 세부 헤더 include

```cpp
// PlayerCharacter.h
class UHealthComponent;   // 전방 선언
class AWeaponBase;

// PlayerCharacter.cpp
#include "Components/HealthComponent.h"
#include "Weapons/WeaponBase.h"
```

---

### 5-3. 파일명

파일명에 클래스 접두사를 붙이지 않는다.

```
✅ PlayerCharacter.h / PlayerCharacter.cpp
❌ APlayerCharacter.h / APlayerCharacter.cpp
```

---

### 5-4. 헤더 내 static 변수 금지

```cpp
// ❌ 헤더
static const FString GlobalMonsterTag = TEXT("Monster");

// ✅ 헤더
extern PENTAGRAM_API const FString GlobalMonsterTag;

// ✅ .cpp
const FString GlobalMonsterTag = TEXT("Monster");
```

---

## 6. 주석 작성법

- **왜(Why)** 이 코드를 작성했는지 설명한다. 무엇을 하는지(What)는 코드 자체로 표현한다.
- 명백한 내용의 중복 주석은 작성하지 않는다.

```cpp
// ❌
// Leaves를 1 증가
++Leaves;

// ✅
// 카운터 공격 판정 구간에서는 일반 피격 무시
if (BossPhase == EBossPhase::CounterWindow)
{
    return 0.f;
}
```

공개 API 함수에는 JavaDoc 스타일 주석을 작성한다.

```cpp
/**
 * 대상 액터에게 최종 데미지를 계산하여 적용합니다.
 *
 * @param DamageAmount    원본 데미지 수치
 * @param DamageType      데미지 유형 (물리/마법/고정)
 * @param DamageCauser    데미지를 유발한 액터
 * @return 실제 적용된 데미지 수치
 */
virtual float ApplyFinalDamage(float DamageAmount, EDamageType DamageType, AActor* DamageCauser);
```

---

## 7. Blueprint 연동 규칙

### 7-1. UPROPERTY 지정자

| 지정자 | 용도 |
|--------|------|
| `EditAnywhere` | CDO + 인스턴스 모두 수정 가능 |
| `EditDefaultsOnly` | CDO(기본값)에서만 수정 가능 |
| `EditInstanceOnly` | 레벨 인스턴스에서만 수정 가능 |
| `VisibleAnywhere` | 읽기 전용으로 표시 |
| `BlueprintReadWrite` | Blueprint에서 읽기/쓰기 |
| `BlueprintReadOnly` | Blueprint에서 읽기 전용 |

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Combat|Stats")
float MaxHealth = 100.f;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
TObjectPtr<UHealthComponent> HealthComponent;
```

**Category 규칙**: `"PT|대분류|소분류"` 형식으로 작성한다.
- 최상위는 반드시 프로젝트 접두사 `PT` 로 시작
- 이후 대분류 → 소분류 순으로 `|` 로 구분

```cpp
// ✅
Category = "PT|Combat"
Category = "PT|Combat|Stats"
Category = "PT|Components"

// ❌
Category = "Combat"         // PT 접두사 없음
Category = "combat|stats"   // 소문자 사용
```

---

### 7-2. UFUNCTION 지정자

| 지정자 | 용도 |
|--------|------|
| `BlueprintCallable` | Blueprint에서 호출 가능 (실행 핀 있음) |
| `BlueprintPure` | 순수 함수 (실행 핀 없음, `const` 필수) |
| `BlueprintImplementableEvent` | Blueprint에서 구현 (C++ 본체 없음) |
| `BlueprintNativeEvent` | C++ 기본 구현, Blueprint 오버라이드 가능 |

```cpp
UFUNCTION(BlueprintCallable, Category = "Combat")
void ActivateSkill(int32 SlotIndex);

UFUNCTION(BlueprintPure, Category = "Combat")
bool IsSkillReady(int32 SlotIndex) const;

UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
void OnSkillActivated(int32 SlotIndex);

UFUNCTION(BlueprintNativeEvent, Category = "Combat")
void OnDeath();
virtual void OnDeath_Implementation();
```

---

### 7-3. TObjectPtr 사용

UObject 계열 멤버 변수는 원시 포인터 대신 `TObjectPtr`을 사용한다.

```cpp
// ✅
UPROPERTY(VisibleAnywhere)
TObjectPtr<USkeletalMeshComponent> CharacterMesh;

// ❌
UPROPERTY(VisibleAnywhere)
USkeletalMeshComponent* CharacterMesh;
```

---

### 7-4. bool 파라미터 대신 Enum 사용

```cpp
// ❌
void SpawnMonster(bool bIsElite, bool bHasArmor, bool bIsRanged);
SpawnMonster(true, false, true);

// ✅
enum class EMonsterFlags : uint8
{
    None    = 0x00,
    Elite   = 0x01,
    Armored = 0x02,
    Ranged  = 0x04,
};
ENUM_CLASS_FLAGS(EMonsterFlags)

void SpawnMonster(EMonsterFlags Flags = EMonsterFlags::None);
SpawnMonster(EMonsterFlags::Elite | EMonsterFlags::Ranged);
```

---

## 8. 폴더 구조

### 8-1. 콘텐츠 폴더

```
Content/
└── Pentagram/
    ├── Core/               # GameMode, PlayerController, GameInstance
    ├── Characters/
    │   ├── Player/
    │   └── Monsters/
    ├── Weapons/
    ├── Skills/
    ├── UI/
    ├── Maps/               # 모든 레벨 파일
    ├── VFX/                # Niagara 시스템
    ├── Audio/
    ├── Art/
    │   └── MaterialLibrary/
    ├── Data/               # DataTable, DataAsset
    └── Developers/         # 개인 작업 폴더 (커밋 금지)
```

---

### 8-2. 폴더 네이밍 규칙

- **PascalCase** 사용
- 공백, 유니코드, 특수문자 사용 금지 (`A-Za-z0-9_`만 허용)
- 사용하지 않는 빈 폴더는 즉시 삭제
- `Assets`, `Meshes`, `Textures` 같은 타입 이름 폴더 금지

---

### 8-3. 소스 코드 폴더

```
Source/Pentagram/
├── Core/
├── Characters/
│   ├── Player/
│   └── Monsters/
├── Components/
├── Weapons/
├── Skills/
├── UI/
├── Data/
└── Utils/
```

---

## 9. 클래스 설계 원칙

### 9-1. 클래스 내 선언 순서

`public` → `protected` → `private` 순서를 따른다.

```cpp
UCLASS()
class PENTAGRAM_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    //~ AActor Interface
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    //~ End AActor Interface

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ActivateSkill(int32 SlotIndex);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Combat|Stats")
    float MaxHealth = 100.f;

protected:
    virtual void InitComponents();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<UHealthComponent> HealthComponent;

private:
    void HandleDeath();

    UPROPERTY()
    float CurrentHealth = 0.f;

    FTimerHandle RespawnTimerHandle;
};
```

---

### 9-2. 컴포넌트 초기화

컴포넌트는 생성자에서 `CreateDefaultSubobject`로 생성한다.

```cpp
APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    SkillComponent  = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComponent"));
}
```

---

### 9-3. 접근 제어

- 멤버 변수는 기본적으로 `private`으로 선언
- `UPROPERTY`로 Blueprint에 노출하는 변수는 `public`/`protected` 허용
- 파생 클래스에서만 접근이 필요한 변수는 `protected` + getter 제공

---

### 9-4. 변수 선언 위치

변수는 사용 직전에 선언하여 의존 거리를 최소화한다.

```cpp
// ✅
const float KnockbackStrength = GetKnockbackStrength();
LaunchCharacter(Direction * KnockbackStrength, true, false);
```

---

### 9-5. UObject는 포인터로 전달

```cpp
// ✅
void RegisterEnemy(AActor* Enemy);

// ❌
void RegisterEnemy(AActor& Enemy);
```
