// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestHostingGameMode.h"
#include "TestHostingCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATestHostingGameMode::ATestHostingGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
