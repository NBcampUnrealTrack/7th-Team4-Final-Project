// Fill out your copyright notice in the Description page of Project Settings.
// 구조체만 모아두는 헤더 파일 특성상 cpp파일은 필요가 없음. (cpp파일을 공백으로 두거나 삭제)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PGItemTypes.generated.h" // 반드시 파일명.generated.h 형식이어야 합니다.


// 아이템 대분류
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    Equipment    UMETA(DisplayName = "Equipment"),
    Consumable   UMETA(DisplayName = "Consumable")
};


// 아이템 소분류
UENUM(BlueprintType)
enum class EItemType : uint8
{
    Weapon       UMETA(DisplayName = "Weapon"),
    Chest        UMETA(DisplayName = "Chest"),
    Potion       UMETA(DisplayName = "Potion")
};


// 아이템 등급
UENUM(BlueprintType)
enum class EItemGrade : uint8
{
    None         UMETA(DisplayName = "None"), // 소비 아이템용
    Common       UMETA(DisplayName = "Common"),
    Rare         UMETA(DisplayName = "Rare")
};


// 기획서의 ItemData 구조체 명세 
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName Item_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Item_Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemCategory Item_Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemType Item_Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemGrade Item_Grade;

    // 기본 성능 (무기: STR, 갑옷: DEF)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Item_Base_Stat;

    // Rare 등급 옵션 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TArray<FString> Item_Bonus_Options;
};

// 인벤토리 한 칸을 담당할 구조체 
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FItemData ItemData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Quantity;

    // 빈 슬롯인지 확인하는 "헬퍼 함수" 
    bool IsEmpty() const { return Quantity <= 0 || ItemData.Item_ID.IsNone(); }

    FInventorySlot() : Quantity(0) {}
};
