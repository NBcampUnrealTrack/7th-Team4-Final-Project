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
        }
    }

    if (RewardData.ItemDropPool.IsEmpty())
    {
        return;
    }

    if (FMath::FRand() > RewardData.ItemDropRate)
    {
        return;
    }

    const FDataTableRowHandle& SelectedHandle = RewardData.ItemDropPool[FMath::RandRange(0, RewardData.ItemDropPool.Num() - 1)];
    const FItemData* ItemRow = SelectedHandle.GetRow<FItemData>(TEXT("SpawnDeathDrops"));
    if (!ItemRow)
    {
        return;
    }

    const TSubclassOf<APTDropItemActorBase> DropClass =
        ResolveItemDropClass(*ItemRow, RewardData.EquipmentDropClass);
    if (!DropClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RewardSubsystem] 드랍 BP를 찾지 못했습니다. ItemID=%s"),
            *ItemRow->Item_ID.ToString());
        return;
    }

    APTDropItemActorBase* DropActor =
        World->SpawnActorDeferred<APTDropItemActorBase>(DropClass, FTransform(DropLocation));
    if (!DropActor)
    {
        return;
    }

    // 인벤토리 드랍과 동일하게 아이템 데이터의 메시와 드랍 크기를 적용합니다.
    DropActor->InitializeDroppedItem(*ItemRow, 1);
    DropActor->FinishSpawning(FTransform(DropLocation));

    UE_LOG(LogTemp, Log, TEXT("[RewardSubsystem] 아이템 드랍: %s (BP=%s)"),
        *ItemRow->Item_Name.ToString(), *GetNameSafe(DropClass));

    /*if (RewardData.EquipmentDropClass && FMath::FRand() <= RewardData.EquipDropRate)
    {
        World->SpawnActor<AActor>(
            RewardData.EquipmentDropClass,
            DropLocation,
            FRotator::ZeroRotator
        );

        UE_LOG(LogTemp, Log, TEXT("[RewardSubsystem] 장비 드랍 스폰"));
    } */
}

TSubclassOf<APTDropItemActorBase> UPTRewardSubsystem::ResolveItemDropClass(
    const FItemData& ItemData,
    TSubclassOf<APTDropItemActorBase> FallbackClass) const
{
    static const TMap<FName, FSoftClassPath> DropClassPaths =
    {
        { TEXT("Weapon_1"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem.BP_DropItem_C")) },
        { TEXT("Weapon_2"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_CurseDestroyer.BP_DropItem_CurseDestroyer_C")) },
        { TEXT("Weapon_3"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_LongSword.BP_DropItem_LongSword_C")) },
        { TEXT("Weapon_4"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_Shovel.BP_DropItem_Shovel_C")) },
        { TEXT("Wand_1"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_Wand.BP_DropItem_Wand_C")) },
        { TEXT("Wand_2"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_BoneStaff.BP_DropItem_BoneStaff_C")) },
        { TEXT("Wand_3"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_RedMageStaff.BP_DropItem_RedMageStaff_C")) },
        { TEXT("Chest_1"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_Chest.BP_DropItem_Chest_C")) },
        { TEXT("Chest_2"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_Cloth1.BP_DropItem_Cloth1_C")) },
        { TEXT("Helmet_1"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_Helmet.BP_DropItem_Helmet_C")) },
        { TEXT("Potion"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_Potion.BP_DropItem_Potion_C")) },
        { TEXT("SkillBook_1"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook_Piercing.BP_DropItem_SkillBook_Piercing_C")) },
        { TEXT("SkillBook_2"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook_FireBall.BP_DropItem_SkillBook_FireBall_C")) },
        { TEXT("SkillBook_3"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook_IceMisile.BP_DropItem_SkillBook_IceMisile_C")) },
        { TEXT("SkillBook_4"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook_SacrificeShield.BP_DropItem_SkillBook_SacrificeShield_C")) },
        { TEXT("SkillBook_5"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook_Meteor.BP_DropItem_SkillBook_Meteor_C")) },
        { TEXT("SkillBook_6"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook_Smash.BP_DropItem_SkillBook_Smash_C")) },
        { TEXT("SkillBook_7"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook_Slash.BP_DropItem_SkillBook_Slash_C")) },
        { TEXT("SkillBook_8"), FSoftClassPath(TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook_Roar.BP_DropItem_SkillBook_Roar_C")) },
    };

    const FSoftClassPath* DropClassPath = DropClassPaths.Find(ItemData.Item_ID);
    if (DropClassPath == nullptr)
    {
        return FallbackClass;
    }

    UClass* LoadedClass = DropClassPath->TryLoadClass<APTDropItemActorBase>();
    if (LoadedClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RewardSubsystem] 전용 드랍 BP 로드 실패. ItemID=%s Path=%s"),
            *ItemData.Item_ID.ToString(), *DropClassPath->ToString());
        return FallbackClass;
    }

    return LoadedClass;
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
