#include "PTInventoryComponent.h"

#include "PTPlayerController.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "Core/Subsystems/PTSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

UPTInventoryComponent::UPTInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // 네트워크 리플리케이트 활성화
    SetIsReplicatedByDefault(true);
}

void UPTInventoryComponent::OnRep_InventorySlots()
{
    BroadcastInventoryChanged();
}

void UPTInventoryComponent::OnRep_QuickSlots()
{
    OnQuickSlotChanged.Broadcast();
}

FName UPTInventoryComponent::GetQuickSlotItemID(int32 QuickIndex) const
{
    return QuickSlots.IsValidIndex(QuickIndex) ? QuickSlots[QuickIndex] : NAME_None;
}

bool UPTInventoryComponent::UseFirstPotion()
{
    // 인벤에서 첫 번째 포션 슬롯을 찾아 바로 사용
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (!InventorySlots[i].IsEmpty() &&
            InventorySlots[i].ItemData.Item_Category == EItemCategory::Consumable)
        {
            UE_LOG(LogTemp, Warning, TEXT("[UseFirstPotion] slot %d 사용"), i);
            return UsePotion(i);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[UseFirstPotion] 소모품 없음"));
    return false;
}
bool UPTInventoryComponent::RegisterConsumableToQuickSlot(int32 QuickIndex, int32 InventorySlotIndex)
{
    if (!QuickSlots.IsValidIndex(QuickIndex)) return false;
    if (!InventorySlots.IsValidIndex(InventorySlotIndex) || InventorySlots[InventorySlotIndex].IsEmpty())
        return false;

    //소모품만 등록 허용
    if (InventorySlots[InventorySlotIndex].ItemData.Item_Category != EItemCategory::Consumable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[퀵슬롯] 소모품만 등록 가능합니다."));
        return false;
    }

    const FName ItemID = InventorySlots[InventorySlotIndex].ItemData.Item_ID;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    if (!Owner->HasAuthority())
    {
        Server_RegisterConsumableToQuickSlot(QuickIndex, ItemID);
        return true;
    }

    QuickSlots[QuickIndex] = ItemID;
    OnRep_QuickSlots(); // 리슨서버 자체 UI 갱신 (서버는 OnRep 자동 호출 안 됨)
    return true;
}

void UPTInventoryComponent::ClearQuickSlot(int32 QuickIndex)
{
    if (!QuickSlots.IsValidIndex(QuickIndex)) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (!Owner->HasAuthority())
    {
        Server_ClearQuickSlot(QuickIndex);
        return;
    }

    QuickSlots[QuickIndex] = NAME_None;
    OnRep_QuickSlots();
}

bool UPTInventoryComponent::UseQuickSlot(int32 QuickIndex)
{
    if (!QuickSlots.IsValidIndex(QuickIndex)) return false;

    const FName ItemID = QuickSlots[QuickIndex];
    if (ItemID.IsNone()) return false;

    // 등록된 소모품이 인벤토리에 실제로 있는지 (소진 시 무시)
    const int32 SlotIndex = FindSameItemSlot(ItemID);
    if (SlotIndex == INDEX_NONE) return false;

    // UsePotion이 내부에서 클라→서버 RPC(Server_UsePotion) + 서버 차감/힐 타이머까지 처리
    return UsePotion(SlotIndex);
}

void UPTInventoryComponent::Server_RegisterConsumableToQuickSlot_Implementation(int32 QuickIndex, FName ItemID)
{
    if (!QuickSlots.IsValidIndex(QuickIndex)) return;

    // 서버 재검증: 인벤토리에 존재하는 소모품 ID인지 (FindSameItemSlot은 Consumable만 매칭)
    if (!ItemID.IsNone() && FindSameItemSlot(ItemID) == INDEX_NONE) return;

    QuickSlots[QuickIndex] = ItemID;
    OnRep_QuickSlots();
}

bool UPTInventoryComponent::Server_RegisterConsumableToQuickSlot_Validate(int32 QuickIndex, FName ItemID)
{
    return QuickIndex >= 0 && QuickIndex < QuickSlotCount;
}

void UPTInventoryComponent::Server_ClearQuickSlot_Implementation(int32 QuickIndex)
{
    if (!QuickSlots.IsValidIndex(QuickIndex)) return;
    QuickSlots[QuickIndex] = NAME_None;
    OnRep_QuickSlots();
}

bool UPTInventoryComponent::Server_ClearQuickSlot_Validate(int32 QuickIndex)
{
    return QuickIndex >= 0 && QuickIndex < QuickSlotCount;
}

void UPTInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // [네트워크 최적화] 슬롯 초기화는 '서버'에서만 수행해도 리플리케이션을 통해 클라이언트에 전달됨.
    AActor* Owner = GetOwner();
    if (Owner != nullptr && Owner->HasAuthority())
    {
        // PostLogin 저장 복원이 BeginPlay보다 먼저 실행될 수 있으므로 기존 슬롯을 덮어쓰지 않는다.
        if (InventorySlots.IsEmpty())
        {
            InventorySlots.Init(FInventorySlot(), MaxSlotCount);
        }
        else if (InventorySlots.Num() != MaxSlotCount)
        {
            InventorySlots.SetNum(MaxSlotCount);
        }
        // 게임 시작 시 30칸의 빈 슬롯을 미리 확보.
        InventorySlots.Init(FInventorySlot(), MaxSlotCount);
        QuickSlots.Init(NAME_None, QuickSlotCount);
    }
}

bool UPTInventoryComponent::RestoreInventorySlots(const TArray<FInventorySlot>& InInventorySlots)
{
    AActor* Owner = GetOwner();
    if (Owner == nullptr || !Owner->HasAuthority())
    {
        return false;
    }

    TArray<FInventorySlot> RestoredSlots;
    RestoredSlots.Init(FInventorySlot(), MaxSlotCount);

    const int32 CopyCount = FMath::Min(InInventorySlots.Num(), MaxSlotCount);
    for (int32 Index = 0; Index < CopyCount; ++Index)
    {
        const FInventorySlot& SavedSlot = InInventorySlots[Index];
        if (SavedSlot.IsEmpty())
        {
            continue;
        }

        FInventorySlot RestoredSlot = SavedSlot;
        RestoredSlot.Quantity = FMath::Max(SavedSlot.Quantity, 1);
        if (RestoredSlot.ItemIconAsset.IsNull())
        {
            RestoredSlot.ItemIconAsset = RestoredSlot.ItemData.Item_Icon;
        }
        if (RestoredSlot.ItemMeshAsset.IsNull())
        {
            RestoredSlot.ItemMeshAsset = RestoredSlot.ItemData.ItemMeshAsset;
        }
        if (RestoredSlot.ArmorChestMeshAsset.IsNull())
        {
            RestoredSlot.ArmorChestMeshAsset = RestoredSlot.ItemData.ArmorChestMeshAsset;
        }

        RestoredSlots[Index] = MoveTemp(RestoredSlot);
    }

    InventorySlots = MoveTemp(RestoredSlots);
    BroadcastInventoryChanged();
    Owner->ForceNetUpdate();
    return true;
}

// 변수 리플리케이트 규칙 정의
void UPTInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // InventorySlots 배열이 서버에서 바뀌면 연결된 모든 클라이언트에게 자동 동기화.
    DOREPLIFETIME(UPTInventoryComponent, InventorySlots);

    DOREPLIFETIME(UPTInventoryComponent, QuickSlots);
}

bool UPTInventoryComponent::TryAddItem(const FItemData& NewItemData, int32 Count)
{
    // [멀티플레이어] 아이템 획득 연산은 무조건 '서버'에서만 수행되어야 합니다.
    AActor* Owner = GetOwner();
    if (Owner == nullptr || !Owner->HasAuthority()) return false;

    // 유효하지 않은 데이터나 수량 방어 코드
    if (NewItemData.Item_ID.IsNone() || Count <= 0) return false;

    // 소비 아이템(포션)은 스택하지 않고 1개당 빈 칸 1개를 차지한다.
    if (NewItemData.Item_Category == EItemCategory::Consumable)
    {
        // 빈 칸이 Count개 이상인지 먼저 검사 (부분 획득 방지)
        if (!CanAddItem(NewItemData, Count))
        {
            UE_LOG(LogTemp, Warning, TEXT("[인벤토리] 빈 칸 부족: %s (%d칸 필요)"),
                *NewItemData.Item_Name.ToString(), Count);
            return false;
        }

        for (int32 n = 0; n < Count; ++n)
        {
            const int32 EmptyIndex = FindEmptySlot(); // 위에서 검증됨 -> 항상 유효
            InventorySlots[EmptyIndex].ItemData      = NewItemData;
            InventorySlots[EmptyIndex].Quantity      = 1;
            InventorySlots[EmptyIndex].ItemIconAsset = NewItemData.Item_Icon;
        }

        UE_LOG(LogTemp, Log, TEXT("[인벤토리] 소비 아이템 %d칸 등록: %s"),
            Count, *NewItemData.Item_Name.ToString());

        BroadcastInventoryChanged();
        PrintInventoryLog();
        NotifyQuestItemCollected(NewItemData, Count);
        SaveOwnerPlayerState();
        return true;
    }

    // 장비 아이템은 기존대로 빈 슬롯 하나에 등록
    int32 EmptyIndex = FindEmptySlot();
    if (EmptyIndex != INDEX_NONE)
    {
        InventorySlots[EmptyIndex].ItemData = NewItemData;
        InventorySlots[EmptyIndex].Quantity = Count;
        InventorySlots[EmptyIndex].ItemIconAsset = NewItemData.Item_Icon;

        UE_LOG(LogTemp, Log, TEXT("[인벤토리] 새 슬롯(%d번)에 아이템 등록: %s (%d개)"),
            EmptyIndex, *NewItemData.Item_Name.ToString(), Count);

        BroadcastInventoryChanged();
        PrintInventoryLog();
        NotifyQuestItemCollected(NewItemData, Count);
        SaveOwnerPlayerState();
        return true;
    }

    // 가방이 가득 차서 추가 실패
    UE_LOG(LogTemp, Warning, TEXT("[인벤토리] 가방 공간이 부족하여 아이템을 추가할 수 없습니다: %s"), *NewItemData.Item_Name.ToString());
    return false;
}

bool UPTInventoryComponent::CanAddItem(const FItemData& NewItemData, int32 Count) const
{
    if (NewItemData.Item_ID.IsNone() || Count <= 0)
    {
        return false;
    }

    // 소비 아이템은 스택하지 않으므로 Count개만큼의 빈 칸이 필요하다.
    if (NewItemData.Item_Category == EItemCategory::Consumable)
    {
        int32 EmptyCount = 0;
        for (const FInventorySlot& Slot : InventorySlots)
        {
            if (Slot.IsEmpty() && ++EmptyCount >= Count)
            {
                return true;
            }
        }
        return false;
    }

    return FindEmptySlot() != INDEX_NONE;
}

int32 UPTInventoryComponent::GetItemCount(FName ItemID) const
{
    if (ItemID.IsNone())
    {
        return 0;
    }

    int32 TotalCount = 0;
    for (const FInventorySlot& Slot : InventorySlots)
    {
        if (!Slot.IsEmpty() && Slot.ItemData.Item_ID == ItemID)
        {
            TotalCount += Slot.Quantity;
        }
    }

    return TotalCount;
}

bool UPTInventoryComponent::RemoveItem(FName ItemID, int32 Count)
{
    AActor* Owner = GetOwner();
    if (Owner == nullptr || !Owner->HasAuthority() || ItemID.IsNone() || Count <= 0)
    {
        return false;
    }

    if (GetItemCount(ItemID) < Count)
    {
        return false;
    }

    int32 RemainingCount = Count;
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (RemainingCount <= 0)
        {
            break;
        }

        if (Slot.IsEmpty() || Slot.ItemData.Item_ID != ItemID)
        {
            continue;
        }

        const int32 RemoveCount = FMath::Min(Slot.Quantity, RemainingCount);
        Slot.Quantity -= RemoveCount;
        RemainingCount -= RemoveCount;

        if (Slot.Quantity <= 0)
        {
            Slot = FInventorySlot();
        }
    }

    BroadcastInventoryChanged();
    PrintInventoryLog();
    SaveOwnerPlayerState();
    return true;
}

bool UPTInventoryComponent::RemoveItemAtSlot(int32 SlotIndex, int32 Count)
{
    AActor* Owner = GetOwner();
    if (Owner == nullptr || !Owner->HasAuthority() ||
        !InventorySlots.IsValidIndex(SlotIndex) ||
        InventorySlots[SlotIndex].IsEmpty() ||
        Count <= 0)
    {
        return false;
    }

    FInventorySlot& Slot = InventorySlots[SlotIndex];
    if (Slot.Quantity < Count)
    {
        return false;
    }

    Slot.Quantity -= Count;
    if (Slot.Quantity <= 0)
    {
        Slot = FInventorySlot();
    }

    BroadcastInventoryChanged();
    PrintInventoryLog();
    SaveOwnerPlayerState();
    return true;
}

void UPTInventoryComponent::NotifyQuestItemCollected(const FItemData& ItemData, int32 Count) const
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = OwnerPawn->GetPlayerState<APTBasePlayerState>();
    if (PlayerState == nullptr)
    {
        return;
    }

    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
    UPTQuestSubsystem* QuestSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTQuestSubsystem>() : nullptr;
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->UpdateQuestProgress(
            PlayerState,
            EPTQuestConditionType::CollectItem,
            ItemData.Item_ID,
            Count);
    }
}

bool UPTInventoryComponent::UsePotion(int32 SlotIndex) // 소모아이템(포션) 사용
{
    // 슬롯 인덱스 유효성 검사 및 빈 슬롯 검사
    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty()) return false;

    // 카테고리가 포션(Consumable)이 맞는지 확인
    if (InventorySlots[SlotIndex].ItemData.Item_Category != EItemCategory::Consumable)
    {
        UE_LOG(LogTemp, Warning, TEXT("소비 아이템이 아닙니다."));
        return false;
    }

    // [멀티플레이어 핵심 분기]
    // 클라이언트가 UI에서 우클릭 등으로 이 함수를 호출한 경우, 직접 개수를 깎으면 데이터 변조 위험이 있으므로 결정권을 서버한테 넘기는 방식
    AActor* Owner = GetOwner();
    if (Owner == nullptr)
    {
        return false;
    }

    // 로컬만 포션 사운드 재생
    if (USoundBase* UseSound = InventorySlots[SlotIndex].ItemData.UseSound.LoadSynchronous())
    {
        UGameplayStatics::PlaySound2D(this, UseSound);
    }

    if (!Owner->HasAuthority())
    {
        // 클라이언트가 서버에게 안전하게 "나 몇 번 슬롯 물약 쓰겠다" 라고 무전(RPC)을 보내고 리턴.
        Server_UsePotion(SlotIndex);
        return true;
    }

    // 소비 아이템 개수 차감
    const FText UsedItemName = InventorySlots[SlotIndex].ItemData.Item_Name;
    InventorySlots[SlotIndex].Quantity--;


    // 수량이 0 이하가 되었다면 완전히 빈 슬롯으로 초기화
    if (InventorySlots[SlotIndex].Quantity <= 0)
    {
        InventorySlots[SlotIndex] = FInventorySlot();
    }

    UE_LOG(LogTemp, Log, TEXT("[Inventory] Used item: %s / Remaining: %d"),
        *UsedItemName.ToString(), InventorySlots[SlotIndex].Quantity);

    BroadcastInventoryChanged();
    PrintInventoryLog();
    SaveOwnerPlayerState();

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return false;
    }

    World->GetTimerManager().ClearTimer(PotionTimerHandle); // 기존에 돌던 포션 타이머가 있다면 초기화
    PotionTickCount = 0;                                         // 틱 카운터 초기화

    // 1초마다 ExecutePotionHealing 함수를 반복 호출 (총 5회)
    PotionTickCount = 0;
    World->GetTimerManager().SetTimer(PotionTimerHandle, this, &UPTInventoryComponent::ExecutePotionHealing, 1.0f, true);

    return true;
}

// Server RPC 실제 실행부
void UPTInventoryComponent::Server_UsePotion_Implementation(int32 SlotIndex)
{
    // 서버 권한으로 진입했으므로 UsePotion 함수를 다시 호출해 서버 데이터를 공식 차감하고 타이머를 켭니다.
    UsePotion(SlotIndex);
}

// Server RPC 검증부 (잘못된 요청이나 핵유저 방지 필터)
bool UPTInventoryComponent::Server_UsePotion_Validate(int32 SlotIndex)
{
    // 음수 인덱스나 가방 크기를 초과하는 기괴한 슬롯 패킷 차단
    if (SlotIndex < 0 || SlotIndex >= MaxSlotCount)
    {
        return false; // 패킷 변조로 판단하고 해당 유저 접속을 강제 종료시킬 수 있음
    }
    return true;
}

void UPTInventoryComponent::ExecutePotionHealing() // 포션 회복
{
    // 이 코드를 실행하는게 서버가 아니라면 함수를 즉시 리턴
    AActor* Owner = GetOwner();
    if (Owner == nullptr || !Owner->HasAuthority()) return;

    UWorld* World = GetWorld();
    if (World == nullptr) return;

    // 이 컴포넌트를 들고 있는 주인(캐릭터) 가져오기
    APTBaseCharacter* OwnerCharacter = Cast<APTBaseCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        World->GetTimerManager().ClearTimer(PotionTimerHandle);
        return;
    }

    PotionTickCount++;

    // 매초 전체 HP의 5%씩 회복
    float HealAmount = OwnerCharacter->MaxHP * 0.05f;

    // 현재 체력이 최대 체력을 넘지 않도록 회복
    OwnerCharacter->CurrentHP = FMath::Min(OwnerCharacter->CurrentHP + HealAmount, OwnerCharacter->MaxHP);

    UE_LOG(LogTemp, Log, TEXT("[포션 틱 %d회차] 5%% 회복 (+%.1f) -> 현재 HP: %.1f / %.1f"),
        PotionTickCount, HealAmount, OwnerCharacter->CurrentHP, OwnerCharacter->MaxHP);

    // 5초(5번 틱)가 지나면 타이머 종료
    if (PotionTickCount >= 5)
    {
        World->GetTimerManager().ClearTimer(PotionTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("[포션 효과 끝남]"));
    }
}

int32 UPTInventoryComponent::FindSameItemSlot(const FName& ItemID) const
{
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        // 빈 슬롯이 아니고, 카테고리가 소비창이며, 아이템 고유 ID가 일치하는지 검사
        if (!InventorySlots[i].IsEmpty() &&
            InventorySlots[i].ItemData.Item_Category == EItemCategory::Consumable &&
            InventorySlots[i].ItemData.Item_ID == ItemID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

int32 UPTInventoryComponent::FindEmptySlot() const
{
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].IsEmpty())
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void UPTInventoryComponent::PrintInventoryLog()
{
    UE_LOG(LogTemp, Log, TEXT("========= 현재 인벤토리 상태 ========="));
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (!InventorySlots[i].IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("슬롯 [%d]: %s | 수량: %d | 타입: %d"),
                i,
                *InventorySlots[i].ItemData.Item_Name.ToString(),
                InventorySlots[i].Quantity,
                (int32)InventorySlots[i].ItemData.Item_Type);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("======================================"));
}

bool UPTInventoryComponent::UseItemAtSlot(int32 SlotIndex)
{
    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty())
        return false;

    switch (InventorySlots[SlotIndex].ItemData.Item_Type)
    {
    case EItemType::Potion:    return UsePotion(SlotIndex);
    case EItemType::SkillBook: return UseSkillBook(SlotIndex);
    default:                   return false;
    }
}

bool UPTInventoryComponent::UseSkillBook(int32 SlotIndex)
{
    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty())
        return false;
    if (InventorySlots[SlotIndex].ItemData.Item_Type != EItemType::SkillBook)
        return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    if (!Owner->HasAuthority())
    {
        Server_UseSkillBook(SlotIndex);
        return true;
    }

    const FName SkillID = InventorySlots[SlotIndex].ItemData.GrantSkillID;
    if (SkillID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[스킬북] GrantSkillID 비어있음"));
        return false;
    }

    UPTPlayerSkillComponent* SkillComp = Owner->FindComponentByClass<UPTPlayerSkillComponent>();
    if (!SkillComp) return false;

    if (SkillComp->IsSkillLearned(SkillID))
    {
        UE_LOG(LogTemp, Warning, TEXT("[스킬북] 이미 습득: %s"), *SkillID.ToString());
        return false;
    }

    if (!SkillComp->LearnSkill(SkillID))
    {
        UE_LOG(LogTemp, Warning, TEXT("[스킬북] 습득 실패(레벨/데이터): %s"), *SkillID.ToString());
        return false;
    }

    RemoveItemAtSlot(SlotIndex, 1); // 성공 시에만 책 1개 소모
    return true;
}

void UPTInventoryComponent::Server_UseSkillBook_Implementation(int32 SlotIndex)
{
    UseSkillBook(SlotIndex);
}

bool UPTInventoryComponent::Server_UseSkillBook_Validate(int32 SlotIndex)
{
    return SlotIndex >= 0 && SlotIndex < MaxSlotCount;
}

void UPTInventoryComponent::BroadcastInventoryChanged()
{
    OnInventoryChanged.Broadcast();
}

void UPTInventoryComponent::SaveOwnerPlayerState() const
{
    const APawn* OwnerPawn = Cast<APawn>(GetOwner());
    APTBasePlayerState* PlayerState =
        OwnerPawn != nullptr ? OwnerPawn->GetPlayerState<APTBasePlayerState>() : nullptr;
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return;
    }

    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    if (SaveSubsystem != nullptr && !SaveSubsystem->SavePlayer(PlayerState))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Save] Failed to save after an inventory change. Player=%s"),
            *PlayerState->GetPlayerName());
    }
}
