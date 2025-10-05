// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StorableItemInterface.h"
#include "Item.generated.h"

USTRUCT(BlueprintType)
struct FGridPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Row;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Column;

    FGridPosition()
        : Row(0), Column(0)
    {
    }

    FGridPosition(int32 InRow, int32 InColumn)
        : Row(InRow), Column(InColumn)
    {
    }
};

/**
 * 
 */
UCLASS()
class EASYINVENTORYSYSTEM_API UItem : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FGridPosition GridPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TScriptInterface<IStorableItemInterface> StoredItem;

    UItem() = default;
};
