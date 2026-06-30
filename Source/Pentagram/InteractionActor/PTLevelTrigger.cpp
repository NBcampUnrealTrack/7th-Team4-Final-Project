

#include "InteractionActor/PTLevelTrigger.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"


APTLevelTrigger::APTLevelTrigger()
{
    // 루트 컴포넌트로 박스 콜리전 생성 및 설정
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;

    // 오버랩 이벤트 바인딩 
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APTLevelTrigger::OnOverlapBegin);
}

void APTLevelTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return; // 데디케이트 서버 검증

    if (ACharacter* Char = Cast<ACharacter>(OtherActor))
    {
        if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
        {
            // 밟은 유저만 타겟 맵으로 즉시 이동 
            if (!TargetMapName.IsEmpty())
            {
                FString TravelURL = TargetMapName;

                if (!TargetActorTag.IsNone())
                {
                        TravelURL += FString::Printf(TEXT("?PlayerActorTag=%s"), *TargetActorTag.ToString());
                }
                 PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
            }
        }
    }
}

void APTLevelTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

