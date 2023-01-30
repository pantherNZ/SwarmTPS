// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseSpawnPoint.generated.h"

UCLASS()
class SWARM_API ABaseSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseSpawnPoint();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	bool CanSpawn();
	bool HasPlayerLOS();

	// Members
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") bool IsEnabled;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") bool BroodmotherSP;
};
