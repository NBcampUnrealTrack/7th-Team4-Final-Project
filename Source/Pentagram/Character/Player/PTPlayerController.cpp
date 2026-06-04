// PTPlayerController.cpp 
#include "Character/Player/PTPlayerController.h"

#include "CommonActivatableWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "PTPlayerCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "UI/Screens/LayOut/PTPrimaryLayout.h"
#include "Skill/PTSkillComponent.h"
#include "Item/PTDropItemActorBase.h"
#include "PTInventoryComponent.h"


APTPlayerController::APTPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
}

void APTPlayerController::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("Controller BeginPlay Called"));

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    UE_LOG(LogTemp, Warning, TEXT("SetInputMode Called"));

    AddUIInputMapping();

    if (PrimaryLayoutClass)
    {
        PrimaryLayout = CreateWidget<UPTPrimaryLayout>(this, PrimaryLayoutClass);
        if (PrimaryLayout)
        {
            PrimaryLayout->AddToPlayerScreen(0);
            if (ULocalPlayer* LP = GetLocalPlayer())
            {
                if (UPTUIManagerSubsystem* UIMgr = LP->GetSubsystem<UPTUIManagerSubsystem>())
                {
                    UIMgr->RegisterPrimaryLayout(PrimaryLayout);
                    PushInitialHUD();
                }
            }
        }
    }
}

void APTPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);
    UE_LOG(LogTemp, Warning, TEXT("AcknowledgePossession Called"));

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (IMC_Default)
        {
            Subsystem->AddMappingContext(IMC_Default, 0);
            UE_LOG(LogTemp, Warning, TEXT("IMC Added"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("IMC_Default is null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Subsystem is null"));
    }
}

void APTPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    UE_LOG(LogTemp, Warning, TEXT("Controller SetupInputComponent Called"));

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (EnhancedInput)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller EnhancedInput Cast Success"));

        if (IA_Move) EnhancedInput->BindAction(IA_Move, ETriggerEvent::Started, this, &APTPlayerController::OnRightClick);

        if (IA_Attack) EnhancedInput->BindAction(IA_Attack, ETriggerEvent::Started, this, &APTPlayerController::OnLeftClick);

        if (IA_Inventory) EnhancedInput->BindAction(IA_Inventory, ETriggerEvent::Started, this, &APTPlayerController::OnInventoryPressed);

        if (IA_Interact)
            UE_LOG(LogTemp, Warning, TEXT("IA_Interact Binding (F Key)"));
            EnhancedInput->BindAction(IA_Interact, ETriggerEvent::Started, this, &APTPlayerController::OnInteractPressed);

        if (IA_Skill1) EnhancedInput->BindAction(IA_Skill1, ETriggerEvent::Started, this, &APTPlayerController::OnSkill1);

        if (IA_Skill2) EnhancedInput->BindAction(IA_Skill2, ETriggerEvent::Started, this, &APTPlayerController::OnSkill2);

        if (IA_Skill3) EnhancedInput->BindAction(IA_Skill3, ETriggerEvent::Started, this, &APTPlayerController::OnSkill3);

        if (IA_Skill4) EnhancedInput->BindAction(IA_Skill4, ETriggerEvent::Started, this, &APTPlayerController::OnSkill4);
    }

    AddUIInputMapping();
}

void APTPlayerController::OnSkill1(const FInputActionValue& Value)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
        PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(0));
}

void APTPlayerController::OnSkill2(const FInputActionValue& Value)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
        PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(1));
}

void APTPlayerController::OnSkill3(const FInputActionValue& Value)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
        PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(2));
}

void APTPlayerController::OnSkill4(const FInputActionValue& Value)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
        PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(3));
}

void APTPlayerController::PlayAttackMontage()
{
    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;
    if (!PlayerCharacter->AttackMontages.IsValidIndex(PlayerCharacter->ComboIndex)) return;

    PlayerCharacter->bIsAttacking = true;
    PlayerCharacter->bCanCombo = false;
    PlayerCharacter->PlayAnimMontage(PlayerCharacter->AttackMontages[PlayerCharacter->ComboIndex]);
    PlayerCharacter->ComboIndex++;
}

void APTPlayerController::OnRightClick(const FInputActionValue& Value)
{
    // [안전장치] 조종중인 캐릭터가 사라져 없거나 이미 죽은 유령 상태라면 마우스 클릭 이동 처리를 완전히 차단 
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()); 
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("현재 조종중인 캐릭터 액터가 월드에 존재하지 않습니다."));
        return;
    }

    if (PC->bIsAttacking) return;

    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    if (HitResult.bBlockingHit)
    {
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.Location); 
    }
}

void APTPlayerController::OnLeftClick(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("OnLeftClick Called"));

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;

    StopMovement();

    // 공격 중이고 콤보 입력 불가 상태면 회전 및 공격 무시
    if (PlayerCharacter->bIsAttacking && !PlayerCharacter->bCanCombo) return;

    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    if (HitResult.bBlockingHit)
    {
        FVector Direction = HitResult.Location - PlayerCharacter->GetActorLocation();
        Direction.Z = 0.f;
        FRotator NewRotation = Direction.Rotation();
        PlayerCharacter->SetActorRotation(NewRotation);
    }

    // 마우스 밑에 있는 오브젝트 스캔
    if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
    {
        // 그 오브젝트가 드롭 아이템 액터인가
        APTDropItemActorBase* TargetItem = Cast<APTDropItemActorBase>(HitResult.GetActor());
        if (TargetItem)
        {
            // 캐릭터와 아이템 간의 평면(2D) 거리 확인
            float Distance2D = FVector::Dist2D(PlayerCharacter->GetActorLocation(), TargetItem->GetActorLocation());
            if (Distance2D <= 100.0f) // 1미터 이내 범위 판정
            {
                // 캐릭터의 인벤토리 컴포넌트를 가져와 아이템 집어넣기
                if (PlayerCharacter->GetInventoryComponent() &&
                    PlayerCharacter->GetInventoryComponent()->TryAddItem(TargetItem->GetItemData(), 1))
                {
                    TargetItem->Destroy(); // 월드에서 아이템에셋 삭제
                    UE_LOG(LogTemp, Log, TEXT("아이템을 획득하였습니다."));

                    return; // 아이템을 주웠으므로 공격 로직을 타지 않도록 리턴
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("아이템이 너무 멀리 있습니다."));
                return; // 거리가 멀어 못 줍는 상태여도 헛공격이 나가지 않도록 잠금
            }
        }
    }

    // 아이템 상호작용이 일어나지 않았다면 콤보 공격 연출 실행
    if (PlayerCharacter->bIsAttacking)
    {
        if (PlayerCharacter->bCanCombo)
        {
            PlayerCharacter->bCanCombo = false;
            PlayAttackMontage();
        }
        return;
    }
    PlayAttackMontage();
}

void APTPlayerController::OnInteractPressed() // F 상호작용 구현부
{
    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (PlayerCharacter)
    {
        // 캐릭터에게 주변 스캔 및 상호작용 처리를 위임합니다.
        // (다음 작업 때 PTPlayerCharacter 클래스 내부에 TryInteract() 함수를 구현해 주면 연동)
        PlayerCharacter->TryInteract();
        UE_LOG(LogTemp, Log, TEXT("컨트롤러: F키 입력 감지 -> 캐릭터에게 상호작용 명령 전달"));
    }
}

void APTPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveUIInputMapping();
    Super::EndPlay(EndPlayReason);
}

void APTPlayerController::OnInventoryPressed()
{
    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP)
    {
        return;
    }

    UPTUIManagerSubsystem* UI = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UI || !InventoryClass)
    {
        return;
    }

    UI->ToggleInventory(InventoryClass);
}

void APTPlayerController::PushInitialHUD()
{
    if (!InitialHUDClass)
    {
        return;
    }

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UPTUIManagerSubsystem* UIMgr = LP->GetSubsystem<UPTUIManagerSubsystem>())
        {
            UIMgr->PushWidget(InitialHUDClass, EPTUILayer::HUD);
        }
    }
}

void APTPlayerController::AddUIInputMapping()
{
    if (bUIInputMappingAdded || !IMC_UI)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!InputSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("EnhancedInputLocalPlayerSubsystem not found."));
        return;
    }

    InputSubsystem->AddMappingContext(IMC_UI, 100);
    bUIInputMappingAdded = true;

    UE_LOG(LogTemp, Warning, TEXT("UI Input Mapping Added: %s / Priority=100"), *IMC_UI->GetName());

}

void APTPlayerController::RemoveUIInputMapping()
{
    if (!bUIInputMappingAdded || !IMC_UI)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!InputSubsystem)
    {
        return;
    }

    InputSubsystem->RemoveMappingContext(IMC_UI);
    bUIInputMappingAdded = false;
}
