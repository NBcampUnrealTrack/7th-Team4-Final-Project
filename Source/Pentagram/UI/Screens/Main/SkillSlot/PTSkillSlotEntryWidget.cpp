#include "PTSkillSlotEntryWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPTSkillSlotEntryWidget::SetIcon(UTexture2D* Icon)
{
    if (!Img_Icon) return;
    if (Icon)
    {
        // 아이콘 표시
        Img_Icon->SetBrushFromTexture(Icon);
        Img_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        // 아이콘 숨김
        Img_Icon->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPTSkillSlotEntryWidget::StartCooldown(float Duration)
{
    // 쿨다운 실행
    SetCooldownRemaining(Duration, Duration);
}

void UPTSkillSlotEntryWidget::SetCooldownRemaining(float Remaining, float Duration)
{
    // 쿨다운 종료
    if (Remaining <= 0.f || Duration <= 0.f)
    {
        bOnCooldown = false;
        ApplyCooldown(0.f);
        return;
    }

    // 목표시간 기록
    CooldownDuration = Duration;
    CooldownEndTime  = GetWorld()->GetTimeSeconds() + Remaining;
    bOnCooldown      = true;

    // UI 갱신
    ApplyCooldown(Remaining);
}

void UPTSkillSlotEntryWidget::SetUsable(bool bUsable)
{
    // BP 이벤트
    OnUsableChanged(bUsable);
}

void UPTSkillSlotEntryWidget::SetKeyLabel(const FText& Key)
{
    // 단축키 설정
    if (Txt_Key) Txt_Key->SetText(Key);
}

void UPTSkillSlotEntryWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // 에디터 초기화
    SetIcon(DefaultIcon);
    SetKeyLabel(KeyText);
    FLinearColor Tint = IconTint;
    if (Tint.A <= 0.f) Tint.A = 1.f;
    if (Img_Icon) Img_Icon->SetColorAndOpacity(Tint);

    // 쿨다운 숨김
    ApplyCooldown(0.f);
}

void UPTSkillSlotEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 준비 완료
    ApplyCooldown(0.f);
}

void UPTSkillSlotEntryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 쿨다운 대기
    if (!bOnCooldown) return;

    // 남은시간 계산
    const float Remaining = CooldownEndTime - GetWorld()->GetTimeSeconds();

    // 쿨다운 끝
    if (Remaining <= 0.f)
    {
        bOnCooldown = false;
        ApplyCooldown(0.f);
        return;
    }

    // UI 갱신
    ApplyCooldown(Remaining);
}

void UPTSkillSlotEntryWidget::ApplyCooldown(float Remaining)
{
    // 진행비율 계산
    const float Percent = (CooldownDuration > 0.f)
        ? FMath::Clamp(Remaining / CooldownDuration, 0.f, 1.f) : 0.f;

    if (PB_Cooldown)
    {
        // 게이지 갱신
        PB_Cooldown->SetPercent(Percent);
        PB_Cooldown->SetVisibility(Remaining > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
    }

    if (Txt_Cooldown)
    {
        if (Remaining > 0.f)
        {
            // 남은 초 표시
            Txt_Cooldown->SetVisibility(ESlateVisibility::HitTestInvisible);
            Txt_Cooldown->SetText(FText::AsNumber(FMath::CeilToInt(Remaining)));
        }
        else
        {
            Txt_Cooldown->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // BP로 값 전달
    OnCooldownUpdated(Percent, Remaining);
}
