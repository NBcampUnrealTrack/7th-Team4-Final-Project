#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTMonsterHealthBarWidget.generated.h"

class APTMonsterCharacter;

UCLASS()
class PENTAGRAM_API UPTMonsterHealthBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    /** 몬스터를 바인딩하고 HP 변경을 구독. 보스 등 파생 위젯도 이 함수를 사용. */
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void SetupMonster(APTMonsterCharacter* InMonster);

    /** OnHPChanged 델리게이트 콜백. */
    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** 새 몬스터가 바인딩된 직후 호출. 파생 클래스에서 추가 델리게이트 바인딩에 사용. */
    virtual void OnMonsterBound(APTMonsterCharacter* InMonster) {}

    /** 몬스터가 언바인딩되기 직전 호출. 파생 클래스에서 추가 델리게이트 해제에 사용. */
    virtual void OnMonsterUnbound(APTMonsterCharacter* InMonster) {}

    /** HP 변경값 적용 직후 호출. 파생 클래스에서 페이즈 계산 등에 사용. */
    virtual void OnHealthChangedNative(float Current, float Max) {}

    /** 현재 바인딩된 몬스터. 파생 클래스 접근 허용. */
    UPROPERTY(Transient)
    TWeakObjectPtr<APTMonsterCharacter> BoundMonster;
};
