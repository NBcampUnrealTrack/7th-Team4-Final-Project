#include "PTPlayerStatusWidget.h"
#include "PTHealthBarwidget.h"
#include "PTExpBarWidget.h"
#include "PTManaBarWidget.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void UPTPlayerStatusWidget::DebugSetAll(float Hp, float MaxHp, float Mp, float MaxMp, float Exp, float ReqExp)
{
    if (HealthBar) HealthBar->SetValue(Hp, MaxHp);
    if (ManaBar)   ManaBar->SetValue(Mp, MaxMp);
    if (ExpBar)    ExpBar->SetValue(Exp, ReqExp);
}

void UPTPlayerStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    TryBindFromOwningPawn();
}

void UPTPlayerStatusWidget::NativeDestruct()
{
    if (BoundPC.IsValid())
    {
        BoundPC->OnPossessedPawnChanged.RemoveDynamic(this, &UPTPlayerStatusWidget::HandlePossessedPawnChanged);
        BoundPC.Reset();
    }

    UnbindFromCharacter();
    Super::NativeDestruct();
}

void UPTPlayerStatusWidget::TryBindFromOwningPawn()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    // 빙의 변경 등록
    if (BoundPC.Get() != PC)
    {
        if (BoundPC.IsValid())
        {
            BoundPC->OnPossessedPawnChanged.RemoveDynamic(this, &UPTPlayerStatusWidget::HandlePossessedPawnChanged);
        }
        BoundPC = PC;
        PC->OnPossessedPawnChanged.AddDynamic(this, &UPTPlayerStatusWidget::HandlePossessedPawnChanged);
    }

    if (APTBaseCharacter* Char = Cast<APTBaseCharacter>(PC->GetPawn()))
    {
        BindToCharacter(Char);
    }
}

void UPTPlayerStatusWidget::BindToCharacter(APTBaseCharacter* InCharacter)
{
    if (!InCharacter || BoundCharacter.Get() == InCharacter) return;

    APTBasePlayerState* PS = InCharacter->GetPlayerState<APTBasePlayerState>();
    if (!PS)
    {
        // PlayerState가 아직 복제/링크 안 됨 → BoundCharacter는 건드리지 않고 다음 틱 재시도
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateUObject(this, &UPTPlayerStatusWidget::TryBindFromOwningPawn));
        return;
    }

    UnbindFromCharacter();
    BoundCharacter = InCharacter;

    if (HealthBar) HealthBar->SetupPlayerState(PS);
    if (ManaBar)   ManaBar->SetupPlayerState(PS);
    if (ExpBar)    ExpBar->SetupPlayerState(PS);

    // 초기 동기화
    bInitialStatsApplied = false;
    RefreshStatsUntilValid();
}

void UPTPlayerStatusWidget::UnbindFromCharacter()
{
    if (!BoundCharacter.IsValid()) return;

    BoundCharacter.Reset();
    // 각 StatBar는 SetupPlayerState(새 PS) 시 이전 PS를 자동 해제하고,
    // 위젯 소멸 시엔 각 바의 NativeDestruct에서 해제됩니다.
}

void UPTPlayerStatusWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    UnbindFromCharacter();

    if (APTBaseCharacter* Char = Cast<APTBaseCharacter>(NewPawn))
    {
        BindToCharacter(Char);
    }
}

void UPTPlayerStatusWidget::RefreshStatsUntilValid()
{
    if (bInitialStatsApplied) return;            // 최초 1회만

    APTBaseCharacter* C = BoundCharacter.Get();
    if (!C) return;

    APTBasePlayerState* PS = C->GetPlayerState<APTBasePlayerState>();
    if (!PS) return;

    if (PS->MaxHP > 0.f)                         // 복제 확인
    {
        PS->BroadcastAllStats();
        bInitialStatsApplied = true;
        return;
    }

    GetWorld()->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateUObject(this, &UPTPlayerStatusWidget::RefreshStatsUntilValid));
}
