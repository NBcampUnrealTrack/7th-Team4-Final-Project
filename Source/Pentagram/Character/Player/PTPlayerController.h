// PTPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "PTPlayerController.generated.h"


class UCommonActivatableWidget;
class UInputAction;
class UInputMappingContext;
class UPTPrimaryLayout;

UCLASS()
class PENTAGRAM_API APTPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    APTPlayerController();

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<class UInputMappingContext> IMC_Default;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<class UInputAction> IA_Move;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Attack;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<class UInputAction> IA_Interact;

    //스킬
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill1;
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill2;
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill3;
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Skill4;

    UFUNCTION(Server, Reliable)
    void Server_SetActorRotation(FRotator NewRotation);

    void OnSkill1(const FInputActionValue& Value);
    void OnSkill2(const FInputActionValue& Value);
    void OnSkill3(const FInputActionValue& Value);
    void OnSkill4(const FInputActionValue& Value);

protected:
    void PlayAttackMontage();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupInputComponent() override;
    virtual void AcknowledgePossession(class APawn* P) override;

private:
    FVector MoveDestination = FVector::ZeroVector;
    bool bMoveToDestination = false;
    static constexpr float AcceptanceRadius = 50.f;

    void OnRightClick(const FInputActionValue& Value);
    void OnLeftClick(const FInputActionValue& Value);
    void OnInteractPressed();

    //UI

public:
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC_UI;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Inventory;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> InitialHUDClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UCommonActivatableWidget> InventoryClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UPTPrimaryLayout> PrimaryLayoutClass;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void OnInventoryPressed();
    void PushInitialHUD();

    UPROPERTY(EditAnywhere, Category = "Input")
    FKey InventoryFallbackKey = EKeys::I;

private:
    void AddUIInputMapping();
    void RemoveUIInputMapping();

    UPROPERTY()
    TObjectPtr<UPTPrimaryLayout> PrimaryLayout;

    bool bUIInputMappingAdded = false;
};
