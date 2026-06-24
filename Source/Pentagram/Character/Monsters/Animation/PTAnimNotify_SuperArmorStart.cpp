#include "Character/Monsters/Animation/PTAnimNotify_SuperArmorStart.h"
#include "Character/Monsters/PTMonsterCharacter.h"

void UPTAnimNotify_SuperArmorStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Monster))
    {
        return;
    }

    Monster->SetSuperArmor(true);

    UE_LOG(LogTemp, Warning, TEXT("SuperArmor Start: %s / Authority: %d"),
        *Monster->GetName(), Monster->HasAuthority());
}
