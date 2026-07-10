#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "UI/Data/PTDelegates.h"
#include "UI/Widget/Widget/Player/PTDamageNumberWidget.h"
#include "PTPlayerController.generated.h"

class UCommonActivatableWidget;
class UInputAction;
class UInputMappingContext;
class UPTPrimaryLayout;
class UPTNPCDialogueWidget;
class APTDropItemActorBase;
class APTMonsterCharacter;
class APTNPCCharacter;
class APTQuestNPCCharacter;
class APTShopNPCCharacter;
class UPTShopWidget;

UCLASS()
class PENTAGRAM_API APTPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    APTPlayerController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupInputComponent() override;
    virtual void AcknowledgePossession(class APawn* P) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void OnSkill1(const FInputActionValue&) { HandleSkillPressed(0); }

    void OnSkill2(const FInputActionValue&) { HandleSkillPressed(1); }

    void OnSkill3(const FInputActionValue&) { HandleSkillPressed(2); }

    void OnSkill4(const FInputActionValue&) { HandleSkillPressed(3); }

    void OnDodge(const FInputActionValue& Value);
    void OnInventoryPressed();
    void OnShopPressed();
    void OnQuestPressed();
    void RegisterNearbyNPC(APTNPCCharacter* NPC);
    void UnregisterNearbyNPC(APTNPCCharacter* NPC);
    void OnSkillWindowPressed();
    void OnCharacterSheetPressed();

    void RestoreGameplayInput();
    void SetGameplayInputBlockedByUI(bool bBlocked);

    UFUNCTION(Client, Reliable)
    void Client_OpenQuestDialogue(
        APTQuestNPCCharacter* QuestNPC,
        TSubclassOf<UPTNPCDialogueWidget> QuestDialogueWidgetClass);

    UFUNCTION(Client, Reliable)
    void Client_OpenShop(
        APTShopNPCCharacter* ShopNPC,
        TSubclassOf<UPTShopWidget> ShopWidgetClass);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerAcceptQuest(APTQuestNPCCharacter* QuestNPC, FName QuestID);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRewardQuest(APTQuestNPCCharacter* QuestNPC, FName QuestID);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerBuyItem(APTShopNPCCharacter* ShopNPC, FName ItemID);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSellItem(APTShopNPCCharacter* ShopNPC, int32 InventorySlotIndex, FName ExpectedItemID, int32 Count);

    UFUNCTION(Server, Reliable)
    void Server_SetActorRotation(FRotator NewRotation);

    // 아이템 획득을 서버에 요청 Server RPC
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TryPickupItem(APTDropItemActorBase* TargetItem);

    UFUNCTION(Server, Reliable)
    void Server_RequestAssignSkillToSlot(FName SkillID, int32 SlotIndex);

    UFUNCTION(Client, Reliable)
    void Client_ShowMonsterHealth(APTMonsterCharacter* Monster);

    // 죽을시 유다이 UI
    UFUNCTION(Client, Reliable)
    void Client_ShowDeathMenu();

    // 부활 요청
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RequestRespawn();

    // [디버그] 즉사
    UFUNCTION(Server, Reliable)
    void Server_DebugKill();

    UFUNCTION(Server, Reliable)
    void Server_SetReady(bool bReady);

    // 로비 채팅 추가
    UFUNCTION(Server, Reliable)
    void Server_SendChatMessage(const FString& Message);

    UFUNCTION(Client, Unreliable)
    void Client_ShowDamageNumber(FVector WorldLocation, float DamageAmount, bool bIsCritical);

    UFUNCTION()
    void RequestEquipItem(int32 InventoryIndex, EItemType EquipType);

    UFUNCTION()
    void RequestUnequipItem(EItemType EquipType, int32 ToInventoryIndex);

    UFUNCTION(Server, Reliable)
    void Server_RequestEquipItem(int32 InventoryIndex, EItemType EquipType);

    UFUNCTION(Server, Reliable)
    void Server_RequestUnequipItem(EItemType EquipType);

    UFUNCTION()
    void RefreshInventoryUI();

    void OnSkill1Released(const FInputActionValue&) { HandleSkillReleased(0); }

    void OnSkill2Released(const FInputActionValue&) { HandleSkillReleased(1); }

    void OnSkill3Released(const FInputActionValue&) { HandleSkillReleased(2); }

    void OnSkill4Released(const FInputActionValue&) { HandleSkillReleased(3); }

    void HandleSkillPressed(int32 SlotIndex);

    void HandleSkillReleased(int32 SlotIndex);

    void BeginSkillAim(int32 SlotIndex);

    void UpdateSkillAim();

    void ConfirmSkillAim();

    void CancelSkillAim();

    bool GetGroundPointUnderCursor(FVector& OutPoint) const;

    AActor* FindTargetUnderCursor(float MaxRange) const;

protected:
    void PlayAttackMontage();

private:
    void RotateTowardsMouse();
    void OnRightClick(const FInputActionValue& Value);
    void OnLeftClick(const FInputActionValue& Value);
    void OnInteractPressed();
    APTNPCCharacter* GetBestNearbyNPC() const;
    void AddUIInputMapping();
    void RemoveUIInputMapping();

    bool IsMouseOverGameplayUI() const;

    // [디버그] 즉사 입력
    void OnDebugKillPressed();

public:
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC_Default;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC_UI;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Move;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Attack;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Interact;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Dodge;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Inventory;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Shop;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Quest;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill1;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill2;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill3;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill4;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_SkillWindow;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_CharacterSheet;
    // [디버그] 즉사 키
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_DebugKill;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> InventoryClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> SkillWindowClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> CharacterSheetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UPTPrimaryLayout> PrimaryLayoutClass;

    UPROPERTY(BlueprintAssignable, Category = "PT|UI")
    FPTOnMonsterTargeted OnMonsterTargeted;

    // 상점 클래스
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> ShopClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPTNPCDialogueWidget> QuestClass;

    // 데스 UI
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> DeathMenuClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UPTDamageNumberWidget> DamageNumberWidgetClass;

    UPROPERTY() TObjectPtr<AActor> CachedTarget = nullptr;

    UPROPERTY() TObjectPtr<class APTSkillIndicatorActor> IndicatorActor;

    UPROPERTY(EditDefaultsOnly, Category="Skill")
    TSubclassOf<class APTSkillIndicatorActor> IndicatorActorClass;

    bool  bIsAiming = false;

    int32 AimingSlotIndex = INDEX_NONE;

    FName AimingSkillID = NAME_None;

private:
    FVector MoveDestination = FVector::ZeroVector;
    bool bMoveToDestination = false;
    static constexpr float AcceptanceRadius = 50.f;

    UPROPERTY()
    TObjectPtr<UPTPrimaryLayout> PrimaryLayout;

    bool bUIInputMappingAdded = false;
    bool bGameplayInputBlockedByUI = false;

    UPROPERTY()
    TArray<TObjectPtr<APTNPCCharacter>> NearbyNPCs;
};
