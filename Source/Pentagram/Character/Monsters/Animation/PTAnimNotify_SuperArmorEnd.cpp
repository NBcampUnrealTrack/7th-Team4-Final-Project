#include "Character/Monsters/Animation/PTAnimNotify_SuperArmorEnd.h"
#include "Character/Monsters/PTMonsterCharacter.h"

void UPTAnimNotify_SuperArmorEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Monster))
    {
        return;
    }

    Monster->SetSuperArmor(false);

    UE_LOG(LogTemp, Warning, TEXT("SuperArmor End: %s / Authority: %d"),
        *Monster->GetName(), Monster->HasAuthority());
}
