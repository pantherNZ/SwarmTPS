// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SwarmGameMode.h"
#include "Swarm.h"
#include "SwarmCharacter.h"

ASwarmGameMode::ASwarmGameMode()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/ThirdPersonCharacter"));
	//if (PlayerPawnBPClass.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}

	CanHighlight = true;
	MonstersHighlighted = 0;
}

void ASwarmGameMode::ServerTravel(FString MapName)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "ServerTravel Called");

	UWorld* World = GetWorld();

	if (IsValid(World))
	{
		World->ServerTravel(MapName, true, false);
	}
}