#include "Core/PTRewardSubsystem.h"
#include "Character/Monsters/PTMonsterRewardData.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Core/PTPlayerLevelSubsystem.h"
#include "Item/PTGoldPickup.h"
#include "Item/PTDropItemActorBase.h"
#include "Item/PTItemTypes.h"
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

void UPTRewardSubsystem::SpawnDeathDrops(APTMonsterCharacter* DeadMonster)
{
    if (!DeadMonster)
    {
        return;
    }

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
        if (!RewardData.ItemRowHandle.DataTable || RewardData.ItemRowHandle.RowName.IsNone())
        {
            return;
        }

        if (!RewardData.ItemRowHandle.DataTable || RewardData.ItemRowHandle.RowName.IsNone())
        {
            return;
        }

        const FItemData* ItemData = RewardData.ItemRowHandle.DataTable->FindRow<FItemData>(RewardData.ItemRowHandle.RowName, TEXT("SpawnDeathDrops"));

        if (!ItemData)
        {
            return;
        }

        APTDropItemActorBase* DropActor = World->SpawnActorDeferred<APTDropItemActorBase>(RewardData.EquipmentDropClass, FTransform(DropLocation));
        if (!DropActor)
        {
            return;
        }

        DropActor->SetItemData(*ItemData);
        DropActor->FinishSpawning(FTransform(DropLocation));
        DropActor->RefreshItemVisual();

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
