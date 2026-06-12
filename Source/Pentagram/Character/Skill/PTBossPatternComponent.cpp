#include "Character/Skill/PTBossPatternComponent.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Skill/PTMonsterSkillComponent.h"
#include "Character/Skill/PTSkillComponent.h"
#include "Engine/DataTable.h"

UPTBossPatternComponent::UPTBossPatternComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPTBossPatternComponent::BeginPlay()
{
    Super::BeginPlay();

    if (APTMonsterCharacter* Owner = Cast<APTMonsterCharacter>(GetOwner()))
    {
        SkillComponent = Owner->SkillComponent;
    }
}

void UPTBossPatternComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (auto& Pair : PatternCooldownTimers)
        {
            World->GetTimerManager().ClearTimer(Pair.Value);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void UPTBossPatternComponent::PreloadAllSkills()
{
    if (!IsValid(BossSkillDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] BossSkillDataTable 미설정"));
        bSkillAssetsLoaded = false;
        return;
    }

    const int32 TotalRows = Phase1SkillRowNames.Num() + Phase2SkillRowNames.Num();
    if (TotalRows <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] Preload 실패 — Phase RowName 목록이 비어 있음"));
        bSkillAssetsLoaded = false;
        return;
    }

    int32 FailCount = 0;

    auto LoadRow = [&FailCount](FPTBossSkillRow* Row, const FName& RowName)
    {
        if (!Row)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BossPattern] Preload 실패 — [%s] RowName 또는 RowStruct 확인"), *RowName.ToString());
            ++FailCount;
            return;
        }

        Row->SkillMontage.LoadSynchronous();
        Row->OverrideMontage.LoadSynchronous();
        Row->SkillEffect.LoadSynchronous();
        Row->SkillSound.LoadSynchronous();
        Row->SkillHitSound.LoadSynchronous();
    };

    for (const FName& RowName : Phase1SkillRowNames)
    {
        LoadRow(BossSkillDataTable->FindRow<FPTBossSkillRow>(RowName, TEXT("")), RowName);
    }

    for (const FName& RowName : Phase2SkillRowNames)
    {
        LoadRow(BossSkillDataTable->FindRow<FPTBossSkillRow>(RowName, TEXT("")), RowName);
    }

    if (FailCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] %d / %d Row 로드 실패 — 일부 스킬이 동작하지 않을 수 있음"),
            FailCount, TotalRows);
    }

    bSkillAssetsLoaded = (FailCount < TotalRows);
}

void UPTBossPatternComponent::ExecuteSkillForPhase(int32 Phase)
{
    if (!bSkillAssetsLoaded)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] 스킬 에셋 미로드 — PreloadAllSkills 호출 확인"));
        return;
    }

    if (!IsValid(SkillComponent))
    {
        return;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    auto [RowName, Row] = PickNextSkill(Phase);
    if (!Row)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[BossPattern] 선택 성공 — %s"), *RowName.ToString());

    FPTSkillActivationRequest Request;
    Request.SkillRowName    = RowName;
    Request.SkillDataTable  = BossSkillDataTable;
    Request.OverrideMontage = Row->OverrideMontage;

    const bool bActivated = SkillComponent->TryActivateSkillChecked(Request);
    if (!bActivated)
    {
        return;
    }

    if (Row->PatternCooldown <= 0.f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    PatternCooldownFlags.Add(RowName, true);
    FTimerHandle& Timer = PatternCooldownTimers.FindOrAdd(RowName);

    UE_LOG(LogTemp, Log, TEXT("[BossPattern] PatternCooldown 시작 — %s (%.1f초)"),
        *RowName.ToString(), Row->PatternCooldown);

    World->GetTimerManager().SetTimer(Timer,
        [this, RowName]()
        {
            PatternCooldownFlags.Add(RowName, false);
            UE_LOG(LogTemp, Log, TEXT("[BossPattern] PatternCooldown 종료 — %s"), *RowName.ToString());
        },
        Row->PatternCooldown, false);
}

TPair<FName, FPTBossSkillRow*> UPTBossPatternComponent::PickNextSkill(int32 Phase)
{
    if (!IsValid(BossSkillDataTable))
    {
        return { NAME_None, nullptr };
    }

    const TArray<FName>& RowNames = (Phase <= 1) ? Phase1SkillRowNames : Phase2SkillRowNames;
    if (RowNames.IsEmpty())
    {
        return { NAME_None, nullptr };
    }

    TArray<TPair<FName, FPTBossSkillRow*>> Candidates;
    float TotalWeight = 0.f;

    for (const FName& RowName : RowNames)
    {
        FPTBossSkillRow* Row = BossSkillDataTable->FindRow<FPTBossSkillRow>(RowName, TEXT("PickNextSkill"));

        if (!Row)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BossPattern] [%s] Row 조회 실패 — 후보에서 제외"), *RowName.ToString());
            continue;
        }

        if (Row->Weight <= 0.f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BossPattern] [%s] Weight <= 0 — 후보에서 제외"), *RowName.ToString());
            continue;
        }

        if (PatternCooldownFlags.FindRef(RowName))
        {
            continue;
        }

        TotalWeight += Row->Weight;
        Candidates.Add({ RowName, Row });
    }

    if (TotalWeight <= 0.f)
    {
        return { NAME_None, nullptr };
    }

    float Rand = FMath::FRandRange(0.f, TotalWeight);
    float Acc  = 0.f;
    for (auto& [RowName, Row] : Candidates)
    {
        Acc += Row->Weight;
        if (Rand <= Acc)
        {
            return { RowName, Row };
        }
    }

    return { NAME_None, nullptr };
}
