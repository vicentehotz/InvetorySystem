// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventorySystemGameMode.h"
#include "InventorySystemCharacter.h"
#include "UObject/ConstructorHelpers.h"

AInventorySystemGameMode::AInventorySystemGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
