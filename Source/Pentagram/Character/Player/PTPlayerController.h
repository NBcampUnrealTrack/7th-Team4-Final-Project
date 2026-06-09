#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
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

    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupInputComponent() override;
    virtual void AcknowledgePossession(class APawn* P) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    void OnSkill1(const FInputActionValue& Value);
    void OnSkill2(const FInputActionValue& Value);
    void OnSkill3(const FInputActionValue& Value);
    void OnSkill4(const FInputActionValue& Value);
    void OnDodge(const FInputActionValue& Value);
    void OnInventoryPressed();
    void PushInitialHUD();

    // ── RPC 함수 ─────────────────────────────────────────────────────────────

    UFUNCTION(Server, Reliable)
    void Server_SetActorRotation(FRotator NewRotation);

    // 아이템 획득을 서버에 요청 Server RPC
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TryPickupItem(APTDropItemActorBase* TargetItem);

protected:
    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    void PlayAttackMontage();

private:
    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    void OnRightClick(const FInputActionValue& Value);
    void OnLeftClick(const FInputActionValue& Value);
    void OnInteractPressed();
    void AddUIInputMapping();
    void RemoveUIInputMapping();

public:
    // ── 멤버 변수 ────────────────────────────────────────────────────────────

    // ── 입력 액션 ────────────────────────────────────────────────────────────

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

    // ── UI ───────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> InitialHUDClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> InventoryClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UPTPrimaryLayout> PrimaryLayoutClass;

private:
    // ── 멤버 변수 (private) ──────────────────────────────────────────────────

    FVector MoveDestination = FVector::ZeroVector;
    bool bMoveToDestination = false;
    static constexpr float AcceptanceRadius = 50.f;

    UPROPERTY()
    TObjectPtr<UPTPrimaryLayout> PrimaryLayout;

    bool bUIInputMappingAdded = false;
};
