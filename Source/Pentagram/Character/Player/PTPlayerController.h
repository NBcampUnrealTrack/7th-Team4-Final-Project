#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "UI/Data/PTDelegates.h"
#include "PTPlayerController.generated.h"

class UCommonActivatableWidget;
class UInputAction;
class UInputMappingContext;
class UPTPrimaryLayout;
class APTDropItemActorBase;

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

    void OnSkill1(const FInputActionValue& Value);
    void OnSkill2(const FInputActionValue& Value);
    void OnSkill3(const FInputActionValue& Value);
    void OnSkill4(const FInputActionValue& Value);
    void OnDodge(const FInputActionValue& Value);
    void OnInventoryPressed();
    void OnShopPressed();


    UFUNCTION(Server, Reliable)
    void Server_SetActorRotation(FRotator NewRotation);

    // 아이템 획득을 서버에 요청 Server RPC
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TryPickupItem(APTDropItemActorBase* TargetItem);

    UFUNCTION(Client, Reliable)
    void Client_ShowMonsterHealth(APTMonsterCharacter* Monster);

    // 죽을시 유다이 UI
    UFUNCTION(Client, Reliable)
    void Client_ShowDeathMenu();

    // 부활 요청
    UFUNCTION(Server, Reliable)
    void Server_RequestRespawn();
protected:
    void PlayAttackMontage();

private:
    void RotateTowardsMouse();
    void OnRightClick(const FInputActionValue& Value);
    void OnLeftClick(const FInputActionValue& Value);
    void OnInteractPressed();
    void AddUIInputMapping();
    void RemoveUIInputMapping();

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

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill1;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill2;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill3;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill4;

    UPROPERTY(EditAnywhere, Category = "Input")
    FKey InventoryFallbackKey = EKeys::I;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> InventoryClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UPTPrimaryLayout> PrimaryLayoutClass;

    UPROPERTY(BlueprintAssignable, Category = "PT|UI")
    FPTOnMonsterTargeted OnMonsterTargeted;

    // 상점 클래스
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> ShopClass;

    // 데스 UI
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> DeathMenuClass;


private:
    FVector MoveDestination = FVector::ZeroVector;
    bool bMoveToDestination = false;
    static constexpr float AcceptanceRadius = 50.f;

    UPROPERTY()
    TObjectPtr<UPTPrimaryLayout> PrimaryLayout;

    bool bUIInputMappingAdded = false;
};
