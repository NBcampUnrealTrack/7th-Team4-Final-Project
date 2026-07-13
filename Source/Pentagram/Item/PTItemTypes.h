// Fill out your copyright notice in the Description page of Project Settings.
// 구조체만 모아두는 헤더 파일 특성상 cpp파일은 필요가 없음. (cpp파일을 공백으로 두거나 삭제)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PTItemTypes.generated.h" // 반드시 파일명.generated.h 형식이어야 합니다.


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
    Helmet       UMETA(DisplayName = "Helmet"),
    Gloves       UMETA(DisplayName = "Gloves"),
    Boots        UMETA(DisplayName = "Boots"),
    Potion       UMETA(DisplayName = "Potion"),
    SkillBook    UMETA(DisplayName = "Skill Book")
};

//무기 소분류
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Hands       UMETA(DisplayName = "Hands"),
    Sword       UMETA(DisplayName = "Sword"),
    Wand         UMETA(DisplayName = "Wand"),
    Bow          UMETA(DisplayName = "Bow"),
    None         UMETA(DisplayName = "None")
};

// 아이템 등급
UENUM(BlueprintType)
enum class EItemGrade : uint8
{
    None         UMETA(DisplayName = "None"),
    Common       UMETA(DisplayName = "Common"),
    Rare         UMETA(DisplayName = "Rare")
};


// 부위별 추가 옵션의 종류, 확률 가중치, 수치 범위를 정하는 구조체
USTRUCT(BlueprintType)
struct FItemOptionPoolData
{
    GENERATED_BODY()

    // 옵션의 종류 (예: "STR", "DEF", "MaxHP")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Option")
    FString OptionType;

    // 이 옵션이 선택될 확률 가중치 (백분율)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Option")
    int32 Weight;

    // 옵션 확정 시 주사위 굴릴 최소 수치 (음수 입력 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Option")
    int32 MinValue;

    // 옵션 확정 시 주사위 굴릴 최대 수치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Option")
    int32 MaxValue;

    // 값을 정하지 않았을 때 기본값
    FItemOptionPoolData() : OptionType(TEXT("STR")), Weight(10), MinValue(1), MaxValue(5) {}
};


// ItemData 구조체 명세
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName Item_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Item_Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Item_Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Skill")
    FName GrantSkillID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemCategory Item_Category = EItemCategory::Equipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemType Item_Type = EItemType::Weapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemGrade Item_Grade = EItemGrade::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EWeaponType WeaponType = EWeaponType::Hands;

    //아이콘 이미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftObjectPtr<UTexture2D> Item_Icon;

    // 기본 성능 (무기/장갑: STR, 갑옷/신발: DEF, 모자: HP 등으로 장비 컴포넌트에서 매칭)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Item_Base_Stat = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Shop",
        meta = (ClampMin = "1", UIMin = "1"))
    int32 BuyPrice = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Shop")
    bool bCanSell = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Shop",
        meta = (EditCondition = "bCanSell", ClampMin = "-1", UIMin = "-1"))
    int32 SellPriceOverride = -1;

    bool CanSell() const;
    int32 GetSellPrice() const;

    // 아이템(혹은 부위)에 붙을 수 있는 랜덤 옵션 후보 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Random Option")
    TArray<FItemOptionPoolData> OptionPool;

    // 아이템이 생성 시, 최대 몇 개의 무작위 옵션이 붙을 수 있는지 지정 (예: 1~2개 랜덤이면 기획에 따라 처리)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Random Option")
    int32 MaxOptionCount = 0;

    // 서버가 주사위를 굴려 확정 지은 최종 추가 옵션 리스트 (런타임 생성 데이터)
    // 음수가 나오면 자동으로 "STR-5", 양수가 나오면 "STR+7" 형태로 저장되어 인벤토리/장비창에 동기화됩니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Runtime Result")
    TArray<FString> Item_Bonus_Options;

    // 무기/장비의 외형을 담당할 스태틱 메시 에셋 경로
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<UStaticMesh> ItemMeshAsset;

    // 방어구 외형용 — 캐릭터 스켈레톤 공유, 리더 포즈로 구동
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<USkeletalMesh> ArmorChestMeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    FVector WeaponRelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    FRotator WeaponRelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    FVector WeaponRelativeScale = FVector(1.f, 1.f, 1.f);

    //거치 상태(Normal) 트랜스폼
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual | Holster")
    FVector WeaponHolsterRelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual | Holster")
    FRotator WeaponHolsterRelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual | Holster")
    FVector WeaponHolsterRelativeScale = FVector(1.f, 1.f, 1.f);
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

    // 빈 슬롯인지 확인하는 헬퍼 함수
    bool IsEmpty() const { return Quantity <= 0 || ItemData.Item_ID.IsNone(); }

    FInventorySlot() : Quantity(0) {}


    // 아이템의 3D 외형
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<UStaticMesh> ItemMeshAsset;

    // UI에 표시될 2D 아이콘 이미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<UTexture2D> ItemIconAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<USkeletalMesh> ArmorChestMeshAsset;
};
