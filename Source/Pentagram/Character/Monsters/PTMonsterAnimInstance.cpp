#include "Character/Monsters/PTMonsterAnimInstance.h"
#include "Character/Monsters/PTMonsterCharacter.h"

void UPTMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(TryGetPawnOwner());
    if (!Monster)
    {
        Speed        = 0.f;
        CurrentState = EMonsterState::Idle;
        bIsDead      = false;
        return;
    }

    Speed        = Monster->GetVelocity().Size2D();
    CurrentState = Monster->GetCurrentState();
    bIsDead      = (CurrentState == EMonsterState::Dead);
}
