// Copyright Epic Games, Inc. All Rights Reserved.

#include "DungeonRoguelikeGameMode.h"
#include "DungeonRoguelikeCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADungeonRoguelikeGameMode::ADungeonRoguelikeGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
