#include "PTAnimNotify_DodgeInvincibleStart.h"

#include "Character/Player/PTPlayerCharacter.h"

void UPTAnimNotify_DodgeInvincibleStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    // 로컬 클라이언트에서만 RPC 호출 (Multicast 몽타주로 재생 중인 다른 클라는 무시)
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!PC || !PC->IsLocallyControlled()) return;

    PC->OnDodgeInvincibleStart();
}
