// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"
#include "SwarmGameMode.generated.h"

UCLASS(minimalapi)
class ASwarmGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASwarmGameMode();

	UFUNCTION(BlueprintCallable, Category = "Server")
	void ServerTravel(FString MapName);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Stats")
	void CheckHighlight();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterOutline") int32 MonstersHighlighted;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterOutline") bool CanHighlight;
};