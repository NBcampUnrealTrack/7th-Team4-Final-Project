#include "PTRewardSubsystem.h"
#include "Character/Monsters/PTMonsterRewardData.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "PTQuestSubsystem.h"
#include "PTPlayerLevelSubsystem.h"
#include "Item/PTDropItemActorBase.h"
#include "Item/PTGoldPickup.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UPTRewardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UPTRewardSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UPTRewardSubsystem::HandleMonsterDeathReward(APTMonsterCharacter* DeadMonster)
{
    if (!IsValid(DeadMonster))
    {
        return;
    }

    if (!HasServerAuthority())
    {
        return;
    }

    GiveExpToContributors(DeadMonster);
    UpdateKillMonsterQuestProgress(DeadMonster);
    SpawnDeathDrops(DeadMonster);

    DeadMonster->ClearExpContributors();
}

void UPTRewardSubsystem::GiveExpToContributors(APTMonsterCharacter* DeadMonster)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        return;
    }

    UPTPlayerLevelSubsystem* LevelSys = GI->GetSubsystem<UPTPlayerLevelSubsystem>();
    if (!LevelSys)
    {
        return;
    }

    const FPTMonsterRewardData RewardData = DeadMonster->GetRewardData();

    for (const TWeakObjectPtr<APTBasePlayerState>& WeakPS : DeadMonster->GetExpContributors())
    {
        APTBasePlayerState* PS = WeakPS.Get();
        if (!IsValid(PS))
        {
            continue;
        }

        LevelSys->AddExp(PS, RewardData.RewardExp);

        UE_LOG(LogTemp, Log, TEXT("[RewardSubsystem] %s에게 경험치 %d 지급"),
            *PS->GetPlayerName(), RewardData.RewardExp);
    }

}

void UPTRewardSubsystem::UpdateKillMonsterQuestProgress(APTMonsterCharacter* DeadMonster)
{
    UWorld* World = GetWorld();
    UGameInstance* GI = World != nullptr ? World->GetGameInstance() : nullptr;
    UPTQuestSubsystem* QuestSubsystem =
        GI != nullptr ? GI->GetSubsystem<UPTQuestSubsystem>() : nullptr;
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    const FName MonsterID = DeadMonster->GetCharacterDataRowName();
    for (const TWeakObjectPtr<APTBasePlayerState>& WeakPS : DeadMonster->GetExpContributors())
    {
        APTBasePlayerState* PS = WeakPS.Get();
        if (!IsValid(PS))
        {
            continue;
        }

        QuestSubsystem->UpdateQuestProgress(PS, EPTQuestConditionType::KillMonster, MonsterID);
    }
}

void UPTRewardSubsystem::SpawnDeathDrops(APTMonsterCharacter* DeadMonster)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FPTMonsterRewardData RewardData = DeadMonster->GetRewardData();
    const FVector DropLocation = DeadMonster->GetActorLocation() + FVector(0.f, 0.f, 50.f);

    if (RewardData.GoldPickupClass)
    {
        APTGoldPickup* GoldActor = World->SpawnActorDeferred<APTGoldPickup>(RewardData.GoldPickupClass, FTransform(DropLocation));

        if (GoldActor)
        {
            const int32 SafeMin = FMath::Min(RewardData.GoldDropMin, RewardData.GoldDropMax);
            const int32 SafeMax = FMath::Max(RewardData.GoldDropMin, RewardData.GoldDropMax);
            const int32 Amount = FMath::RandRange(SafeMin, SafeMax);

            GoldActor->SetGoldAmount(Amount);
            GoldActor->FinishSpawning(FTransform(DropLocation));

            UE_LOG(LogTemp, Log, TEXT("[RewardSubsystem] 골드 픽업 스폰 — %d골드"), Amount);
        }
    }

    if (RewardData.EquipmentDropClass && FMath::FRand() <= RewardData.EquipDropRate)
    {
        World->SpawnActor<AActor>(
            RewardData.EquipmentDropClass,
            DropLocation,
            FRotator::ZeroRotator
        );

        UE_LOG(LogTemp, Log, TEXT("[RewardSubsystem] 장비 드랍 스폰"));
    }
}

bool UPTRewardSubsystem::HasServerAuthority() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    return World->GetNetMode() != NM_Client;
}
