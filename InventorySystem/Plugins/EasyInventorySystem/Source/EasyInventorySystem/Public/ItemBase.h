// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemBase.generated.h"

UCLASS(BlueprintType, Abstract)
class EASYINVENTORYSYSTEM_API UItemBase : public UObject
{
	GENERATED_BODY()

public:
	/** Nome e descrição */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText Name;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText Description;

	/** Icone para UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) UTexture2D* Icon;

	/** Máximo de unidades por stack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) int32 MaxStackSize = 1;
};
