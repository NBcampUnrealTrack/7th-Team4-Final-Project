// Fill out your copyright notice in the Description page of Project Settings.


#include "PTPlayerStatusWidget.h"
#include "PTHealthBarwidget.h"
#include "PTExpBarWidget.h"
#include "PTManaBarWidget.h"
#include "Character/PTBaseCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void UPTPlayerStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 위젯 생성 시 플레이어 폰 바인딩 시도
    TryBindFromOwningPawn();
}

void UPTPlayerStatusWidget::NativeDestruct()
{
    // 위젯 소멸 시 컨트롤러 델리게이트 해제
    if (BoundPC.IsValid())
    {
        BoundPC->OnPossessedPawnChanged.RemoveDynamic(this, &UPTPlayerStatusWidget::HandlePossessedPawnChanged);
        BoundPC.Reset();
    }

    // 캐릭터 바인딩 해제
    UnbindFromCharacter();
    Super::NativeDestruct();
}

void UPTPlayerStatusWidget::TryBindFromOwningPawn()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    // 빙의(Possess) 변경 이벤트 1회 등록 (멀티플레이/로딩 지연 대비)
    if (BoundPC.Get() != PC)
    {
        if (BoundPC.IsValid())
        {
            BoundPC->OnPossessedPawnChanged.RemoveDynamic(this, &UPTPlayerStatusWidget::HandlePossessedPawnChanged);
        }
        BoundPC = PC;
        PC->OnPossessedPawnChanged.AddDynamic(this, &UPTPlayerStatusWidget::HandlePossessedPawnChanged);
    }

    // 폰이 이미 존재하면 즉시 바인딩 (없으면 위에서 등록한 이벤트가 나중에 처리함)
    if (APTBaseCharacter* Char = Cast<APTBaseCharacter>(PC->GetPawn()))
    {
        BindToCharacter(Char);
    }
}

void UPTPlayerStatusWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    // 조종하는 폰이 바뀌면 기존 해제 후 새 폰에 바인딩
    UnbindFromCharacter();

    if (APTBaseCharacter* Char = Cast<APTBaseCharacter>(NewPawn))
    {
        BindToCharacter(Char);
    }
}

void UPTPlayerStatusWidget::BindToCharacter(APTBaseCharacter* InCharacter)
{
    // 유효성 검사 및 중복 바인딩 방지
    if (!InCharacter || BoundCharacter.Get() == InCharacter)
    {
        return;
    }

    UnbindFromCharacter();
    BoundCharacter = InCharacter;

    // [TODO] 캐릭터 파트 완성 시 주석 해제: 스탯 변경 이벤트 연결 및 초기 UI 셋팅
    // if (HealthBar)
    // {
    //     InCharacter->OnHealthChanged.AddDynamic(HealthBar, &UPTHealthbarwidget::HandleHealthChanged);
    // }
    // if (ManaBar)
    // {
    //     InCharacter->OnManaChanged.AddDynamic(ManaBar, &UPTManaBarWidget::HandleManaChanged);
    // }
    // if (ExpBar)
    // {
    //     InCharacter->OnExpChanged.AddDynamic(ExpBar, &UPTExpBarWidget::HandleExpChanged);
    //     InCharacter->OnLevelChanged.AddDynamic(ExpBar, &UPTExpBarWidget::HandleLevelChanged);
    // }

    // if (HealthBar) HealthBar->SetValueInstant(InCharacter->GetHealth(), InCharacter->GetMaxHealth());
    // if (ManaBar)   ManaBar->SetValueInstant(InCharacter->GetMana(),     InCharacter->GetMaxMana());
    // if (ExpBar)    ExpBar->SetValueInstant(InCharacter->GetExp(),       InCharacter->GetRequiredExp());
}

void UPTPlayerStatusWidget::UnbindFromCharacter()
{
    // 바인딩된 캐릭터가 없으면 무시
    if (!BoundCharacter.IsValid())
    {
        return;
    }

    APTBaseCharacter* C = BoundCharacter.Get();

    // [TODO] 캐릭터 파트 완성 시 주석 해제: 스탯 변경 이벤트 연결 해제
    // if (HealthBar)
    // {
    //     C->OnHealthChanged.RemoveDynamic(HealthBar, &UPTHealthBarWidget::HandleHealthChanged);
    // }
    // if (ManaBar)
    // {
    //     C->OnManaChanged.RemoveDynamic(ManaBar, &UPTManaBarWidget::HandleManaChanged);
    // }
    // if (ExpBar)
    // {
    //     C->OnExpChanged.RemoveDynamic(ExpBar, &UPTExpBarWidget::HandleExpChanged);
    //     C->OnLevelChanged.RemoveDynamic(ExpBar, &UPTExpBarWidget::HandleLevelChanged);
    // }

    // 참조 초기화
    BoundCharacter.Reset();
}

void UPTPlayerStatusWidget::DebugSetAll(float Hp, float MaxHp, float Mp, float MaxMp, float Exp, float ReqExp)
{
    // 디버그 테스트용: 모든 스탯 바 UI 강제 업데이트
    if (HealthBar) HealthBar->SetValue(Hp, MaxHp);
    if (ManaBar)   ManaBar->SetValue(Mp, MaxMp);
    if (ExpBar)    ExpBar->SetValue(Exp, ReqExp);
}
