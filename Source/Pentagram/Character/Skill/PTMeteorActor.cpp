#include "PTMeteorActor.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

APTMeteorActor::APTMeteorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
}

void APTMeteorActor::InitMeteor(APTPlayerCharacter* InAttacker, const FVector& InTargetGround, float InFallHeight,
    float InFallSpeed, float InDamageRadius, float InDamageMultiplier, bool bInApplyDamage, UNiagaraSystem* InImpactVFX,
    USoundBase* InImpactSound, const FPTHitInfo& InHitTemplate)
{
    Attacker         = InAttacker;
    TargetGround     = InTargetGround;
    FallSpeed        = InFallSpeed;
    DamageRadius     = InDamageRadius;
    DamageMultiplier = InDamageMultiplier;
    bApplyDamage     = bInApplyDamage;
    ImpactVFX        = InImpactVFX;
    ImpactSound      = InImpactSound;
    HitTemplate      = InHitTemplate;

    // 착지점 위 하늘에서 시작
    SetActorLocation(TargetGround + FVector(0.f, 0.f, InFallHeight));
}

void APTMeteorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bImpacted) return;

    FVector Loc = GetActorLocation();
    Loc.Z -= FallSpeed * DeltaTime;

    if (Loc.Z <= TargetGround.Z)
    {
        Loc.Z = TargetGround.Z;
        SetActorLocation(Loc);
        OnImpact();
        return;
    }
    SetActorLocation(Loc);
}

void APTMeteorActor::OnImpact()
{
    if (bImpacted) return;
    bImpacted = true;

    // 연출은 모든 머신에서
    if (ImpactVFX)
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactVFX, TargetGround, FRotator::ZeroRotator);
    if (ImpactSound)
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, TargetGround);

    // 데미지는 서버만
    if (bApplyDamage)
    {
        APTPlayerCharacter* AttackerPtr = Attacker.Get();
        if (AttackerPtr)
        {
            TArray<AActor*> HitActors;
            TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
            ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

            UKismetSystemLibrary::SphereOverlapActors(
                GetWorld(), TargetGround, DamageRadius,
                ObjectTypes, nullptr, TArray<AActor*>{AttackerPtr}, HitActors);

            const float FinalDamage = AttackerPtr->GetTotalAttack() * DamageMultiplier;

            for (AActor* HitActor : HitActors)
            {
                APTBaseCharacter* Target = Cast<APTBaseCharacter>(HitActor);
                if (!Target || Cast<APTPlayerCharacter>(Target)) continue;

                FPTHitInfo HitInfo   = HitTemplate;
                HitInfo.Attacker     = AttackerPtr;
                // 착지 중심에서 바깥으로 밀어내는 넉백
                HitInfo.HitDirection = (Target->GetActorLocation() - TargetGround).GetSafeNormal();

                Target->ApplyDamageWithHit(FinalDamage, AttackerPtr, HitInfo);
            }
        }
    }

    // VFX가 다 재생되도록 약간 딜레이 후 소멸
    SetActorTickEnabled(false);
    SetLifeSpan(2.0f);
}

