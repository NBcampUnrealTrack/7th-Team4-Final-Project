// Fill out your copyright notice in the Description page of Project Settings.

#include "PTCharacterSheetWidget.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Layout/SlateRect.h"
#include "GameFramework/PlayerController.h"
#include "Character/Player/PTBasePlayerState.h" // 실제 경로에 맞게 수정

void UPTCharacterSheetWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TryBindFromOwningPlayerState();
}

void UPTCharacterSheetWidget::NativeDestruct()
{
    UnbindFromPlayerState();

    Super::NativeDestruct();
}

void UPTCharacterSheetWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    // 아직 못 묶었으면 재시도 (PlayerState가 늦게 준비되는 경우 대비)
    if (!BoundPlayerState.IsValid())
    {
        TryBindFromOwningPlayerState();
    }

    // 창을 열 때마다 최신값으로 전부 리프레시 (HP/MP/공격력/방어력/치명타/이동속도/레벨)
    if (APTBasePlayerState* PS = BoundPlayerState.Get())
    {
        PS->BroadcastAllStats();
    }

    // 이름은 델리게이트가 없어 매번 직접 읽음
    RefreshPlayerName();
}

void UPTCharacterSheetWidget::TryBindFromOwningPlayerState()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    APTBasePlayerState* PS = PC->GetPlayerState<APTBasePlayerState>();
    if (!PS)
    {
        return;
    }

    BindToPlayerState(PS);
}

void UPTCharacterSheetWidget::BindToPlayerState(APTBasePlayerState* InPS)
{
    if (BoundPlayerState.Get() == InPS)
    {
        return;
    }

    UnbindFromPlayerState();

    if (!InPS)
    {
        return;
    }

    BoundPlayerState = InPS;

    InPS->OnHealthChanged.AddUniqueDynamic(this, &UPTCharacterSheetWidget::HandleHealthChanged);
    InPS->OnManaChanged.AddUniqueDynamic(this, &UPTCharacterSheetWidget::HandleManaChanged);
    InPS->OnAttackChanged.AddUniqueDynamic(this, &UPTCharacterSheetWidget::HandleAttackChanged);
    InPS->OnDefenseChanged.AddUniqueDynamic(this, &UPTCharacterSheetWidget::HandleDefenseChanged);
    InPS->OnCriticalChanged.AddUniqueDynamic(this, &UPTCharacterSheetWidget::HandleCriticalChanged);
    InPS->OnMoveSpeedChanged.AddUniqueDynamic(this, &UPTCharacterSheetWidget::HandleMoveSpeedChanged);
    InPS->OnLevelChanged.AddUniqueDynamic(this, &UPTCharacterSheetWidget::HandleLevelChanged);

    // 바인딩 시점 값 1회 반영 (델리게이트는 값이 변할 때만 브로드캐스트되므로)
    HandleHealthChanged(InPS->CurrentHP, InPS->MaxHP);
    HandleManaChanged(InPS->CurrentMP, InPS->MaxMP);
    HandleAttackChanged(InPS->BaseAtk);
    HandleDefenseChanged(InPS->BaseDef);
    HandleCriticalChanged(InPS->CriticalChance, InPS->CriticalATK);
    HandleMoveSpeedChanged(InPS->MoveSpeed);
    HandleLevelChanged(InPS->PlayerLevel);
    RefreshPlayerName();
}

void UPTCharacterSheetWidget::UnbindFromPlayerState()
{
    if (APTBasePlayerState* PS = BoundPlayerState.Get())
    {
        PS->OnHealthChanged.RemoveDynamic(this, &UPTCharacterSheetWidget::HandleHealthChanged);
        PS->OnManaChanged.RemoveDynamic(this, &UPTCharacterSheetWidget::HandleManaChanged);
        PS->OnAttackChanged.RemoveDynamic(this, &UPTCharacterSheetWidget::HandleAttackChanged);
        PS->OnDefenseChanged.RemoveDynamic(this, &UPTCharacterSheetWidget::HandleDefenseChanged);
        PS->OnCriticalChanged.RemoveDynamic(this, &UPTCharacterSheetWidget::HandleCriticalChanged);
        PS->OnMoveSpeedChanged.RemoveDynamic(this, &UPTCharacterSheetWidget::HandleMoveSpeedChanged);
        PS->OnLevelChanged.RemoveDynamic(this, &UPTCharacterSheetWidget::HandleLevelChanged);
    }

    BoundPlayerState.Reset();
}

void UPTCharacterSheetWidget::HandleHealthChanged(float CurrentHP, float MaxHP)
{
    if (Txt_HP)
    {
        Txt_HP->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHP, MaxHP)));
    }
}

void UPTCharacterSheetWidget::HandleManaChanged(float CurrentMP, float MaxMP)
{
    if (Txt_MP)
    {
        Txt_MP->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentMP, MaxMP)));
    }
}

void UPTCharacterSheetWidget::HandleAttackChanged(float NewAttack)
{
    // ── 아이템 공격력 보너스 (미구현) ──────────────────────────
    // 장착 슬롯은 인벤토리 컴포넌트에 있으나 아이템→스탯 반영 로직은
    // 아이템 쪽에서 처리하도록 설계 예정이라 아직 미연동.
    // 연동되면 여기서 보너스 값만 구해서 두 번째 인자로 넘기면 됨.
    const float ItemAtkBonus = 0.f;

    if (Txt_Atk)
    {
        Txt_Atk->SetText(FormatStatWithItemBonus(NewAttack, ItemAtkBonus));
    }
}

void UPTCharacterSheetWidget::HandleDefenseChanged(float NewDefense)
{
    // ── 아이템 방어력 보너스 (미구현) ──────────────────────────
    const float ItemDefBonus = 0.f;

    if (Txt_Def)
    {
        Txt_Def->SetText(FormatStatWithItemBonus(NewDefense, ItemDefBonus));
    }
}

void UPTCharacterSheetWidget::HandleCriticalChanged(float CriticalChance, float CriticalDamage)
{
    if (Txt_CritChance)
    {
        Txt_CritChance->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), CriticalChance * 100.f)));
    }

    if (Txt_CritDamage)
    {

        Txt_CritDamage->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), CriticalDamage * 100.f)));
    }
}

void UPTCharacterSheetWidget::HandleMoveSpeedChanged(float NewMoveSpeed)
{
    if (Txt_MoveSpeed)
    {
        Txt_MoveSpeed->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), NewMoveSpeed)));
    }
}

void UPTCharacterSheetWidget::HandleLevelChanged(int32 NewLevel)
{
    if (Txt_Level)
    {
        Txt_Level->SetText(FText::AsNumber(NewLevel));
    }
}

void UPTCharacterSheetWidget::RefreshPlayerName()
{
    APTBasePlayerState* PS = BoundPlayerState.Get();
    if (!PS || !Txt_Name)
    {
        return;
    }


    Txt_Name->SetText(FText::FromString(PS->GetPlayerName()));
}

FText UPTCharacterSheetWidget::FormatStatWithItemBonus(float BaseValue, float ItemBonus) const
{
    if (FMath::IsNearlyZero(ItemBonus))
    {
        return FText::FromString(FString::Printf(TEXT("%.0f"), BaseValue));
    }

    return FText::FromString(FString::Printf(TEXT("%.0f (+%.0f)"), BaseValue, ItemBonus));
}

bool UPTCharacterSheetWidget::IsScreenPositionOverContent_Implementation(const FVector2D& ScreenPosition) const
{

    if (Border_Background)
    {
        return Border_Background->GetCachedGeometry().IsUnderLocation(ScreenPosition);
    }


    const UWidget* ContentWidgets[] = {
        Txt_Name, Txt_Level, Txt_HP, Txt_MP,
        Txt_Atk, Txt_Def, Txt_CritChance, Txt_CritDamage, Txt_MoveSpeed
    };

    float MinX = TNumericLimits<float>::Max();
    float MinY = TNumericLimits<float>::Max();
    float MaxX = TNumericLimits<float>::Lowest();
    float MaxY = TNumericLimits<float>::Lowest();
    bool bHasAny = false;

    for (const UWidget* Widget : ContentWidgets)
    {
        if (!Widget)
        {
            continue;
        }

        const FSlateRect Rect = Widget->GetCachedGeometry().GetLayoutBoundingRect();
        MinX = FMath::Min(MinX, Rect.Left);
        MinY = FMath::Min(MinY, Rect.Top);
        MaxX = FMath::Max(MaxX, Rect.Right);
        MaxY = FMath::Max(MaxY, Rect.Bottom);
        bHasAny = true;
    }

    if (!bHasAny)
    {
        return false;
    }

    constexpr float ContentPaddingPx = 24.f;
    const FSlateRect PaddedRect(MinX - ContentPaddingPx, MinY - ContentPaddingPx, MaxX + ContentPaddingPx, MaxY + ContentPaddingPx);

    return PaddedRect.ContainsPoint(ScreenPosition);
}
