// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PBMovementsGameMode.h"
#include "PBMovementsCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/GameFramework/HUD.h"
APBMovementsGameMode::APBMovementsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	static ConstructorHelpers::FClassFinder<AHUD> HudBPClass(TEXT("/Game/HUD/PBHUD"));

	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	if (HudBPClass.Class != NULL)
	{
		HUDClass = HudBPClass.Class;
	}
}
