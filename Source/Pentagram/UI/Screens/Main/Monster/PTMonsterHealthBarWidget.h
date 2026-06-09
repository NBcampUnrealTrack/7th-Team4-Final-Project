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
<<<<<<< HEAD
    // 몬스터 바인딩
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void SetupMonster(APTMonsterCharacter* InMonster);

    // HP 콜백
=======
    /** 몬스터를 바인딩하고 HP 변경을 구독. 보스 등 파생 위젯도 이 함수를 사용. */
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void SetupMonster(APTMonsterCharacter* InMonster);

    /** OnHPChanged 델리게이트 콜백. */
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

protected:
<<<<<<< HEAD
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
=======
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** 새 몬스터가 바인딩된 직후 호출. 파생 클래스에서 추가 델리게이트 바인딩에 사용. */
    virtual void OnMonsterBound(APTMonsterCharacter* InMonster) {}

    /** 몬스터가 언바인딩되기 직전 호출. 파생 클래스에서 추가 델리게이트 해제에 사용. */
    virtual void OnMonsterUnbound(APTMonsterCharacter* InMonster) {}

    /** HP 변경값 적용 직후 호출. 파생 클래스에서 페이즈 계산 등에 사용. */
    virtual void OnHealthChangedNative(float Current, float Max) {}

    /** 현재 바인딩된 몬스터. 파생 클래스 접근 허용. */
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
    UPROPERTY(Transient)
    TWeakObjectPtr<APTMonsterCharacter> BoundMonster;
};
