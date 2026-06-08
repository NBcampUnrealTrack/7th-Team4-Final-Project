#include "PTPlayerStatusWidget.h"
#include "Player/PTHealthBarwidget.h"
#include "Player/PTExpBarWidget.h"
#include "Player/PTManaBarWidget.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

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

    // 빙의 변경 이벤트 1회 등록 (멀티플레이/로딩 지연 대비)
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

void UPTPlayerStatusWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    UnbindFromCharacter();

    if (APTBaseCharacter* Char = Cast<APTBaseCharacter>(NewPawn))
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
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateUObject(this, &UPTPlayerStatusWidget::TryBindFromOwningPawn));
        return;
    }

    UnbindFromCharacter();
    BoundCharacter = InCharacter;

    if (HealthBar) HealthBar->SetupPlayerState(PS);
    if (ManaBar)   ManaBar->SetupPlayerState(PS);
    if (ExpBar)    ExpBar->SetupPlayerState(PS);

    // 초기 동기화 재시작: 스탯 값이 복제될 때까지 폴링
    bInitialStatsApplied = false;
    RefreshStatsUntilValid();
}

void UPTPlayerStatusWidget::RefreshStatsUntilValid()
{
    if (bInitialStatsApplied) return;            // 최초 1회 성공 후 영구 종료 (죽음/런타임과 무관)

    APTBaseCharacter* C = BoundCharacter.Get();
    if (!C) return;

    APTBasePlayerState* PS = C->GetPlayerState<APTBasePlayerState>();
    if (!PS) return;

    if (PS->MaxHP > 0.f)                         // 스탯 복제 도착 확인
    {
        PS->BroadcastAllStats();
        bInitialStatsApplied = true;
        return;
    }

    GetWorld()->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateUObject(this, &UPTPlayerStatusWidget::RefreshStatsUntilValid));
}

void UPTPlayerStatusWidget::UnbindFromCharacter()
{
    if (!BoundCharacter.IsValid()) return;

    BoundCharacter.Reset();
}

void UPTPlayerStatusWidget::DebugSetAll(float Hp, float MaxHp, float Mp, float MaxMp, float Exp, float ReqExp)
{
    if (HealthBar) HealthBar->SetValue(Hp, MaxHp);
    if (ManaBar)   ManaBar->SetValue(Mp, MaxMp);
    if (ExpBar)    ExpBar->SetValue(Exp, ReqExp);
}
