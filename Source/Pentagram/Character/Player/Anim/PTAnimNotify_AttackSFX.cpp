#include "PTAnimNotify_AttackSFX.h"

#include "Kismet/GameplayStatics.h"

void UPTAnimNotify_AttackSFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !SoundAsset) return;

    UGameplayStatics::PlaySoundAtLocation(
        MeshComp->GetWorld(),
        SoundAsset,
        MeshComp->GetComponentLocation()
    );
}
