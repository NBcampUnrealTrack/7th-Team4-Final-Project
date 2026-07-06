#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "PTSkillDragDropOperation.generated.h"

// 스킬 드래그 페이로드
UCLASS()
class PENTAGRAM_API UPTSkillDragDropOperation : public UDragDropOperation
{
    GENERATED_BODY()

public:
    // 드래그 중인 스킬
    UPROPERTY(BlueprintReadWrite, Category = "PT|Skill")
    FName SkillID = NAME_None;

    // 원형 드래그 비주얼용 아이콘
    UPROPERTY(BlueprintReadWrite, Category = "PT|Skill")
    TSoftObjectPtr<UTexture2D> SkillIcon;
};
