#include "Character/Monsters/Animation/PTAnimNotify_PlaySound.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void UPTAnimNotify_PlaySound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!IsValid(MeshComp))
    {
        return;
    }

    USoundBase* LoadedSound = Sound.LoadSynchronous();
    if (!IsValid(LoadedSound))
    {
        return;
    }

    FVector Location = MeshComp->GetComponentLocation();
    if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
    {
        Location = MeshComp->GetSocketLocation(SocketName);
    }

    UGameplayStatics::SpawnSoundAtLocation(
        MeshComp->GetWorld(), LoadedSound, Location,
        FRotator::ZeroRotator, VolumeMultiplier
    );
}
