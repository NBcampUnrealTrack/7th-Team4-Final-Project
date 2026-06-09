#pragma once

#include "CoreMinimal.h"
#include "../PTStatBarWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTMonsterHealthBarWidget.generated.h"

class APTMonsterCharacter;

UCLASS()
class PENTAGRAM_API UPTMonsterHealthBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    // 몬스터 바인딩
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void SetupMonster(APTMonsterCharacter* InMonster);

    // HP 콜백
    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 바인딩 직후
    virtual void OnMonsterBound(APTMonsterCharacter* InMonster) {}

    // 해제 직전
    virtual void OnMonsterUnbound(APTMonsterCharacter* InMonster) {}

    // HP 적용 후
    virtual void OnHealthChangedNative(float Current, float Max) {}

    // 바인딩 대상
    UPROPERTY(Transient)
    TWeakObjectPtr<APTMonsterCharacter> BoundMonster;
};
