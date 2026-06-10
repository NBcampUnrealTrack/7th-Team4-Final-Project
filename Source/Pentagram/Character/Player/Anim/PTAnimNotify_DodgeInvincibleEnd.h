#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_DodgeInvincibleEnd.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_DodgeInvincibleEnd : public UAnimNotify
{
    GENERATED_BODY()

public:
    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
