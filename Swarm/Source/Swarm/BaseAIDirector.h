// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Utility.h"
#include "BaseSpawnPoint.h"
#include "BaseAIDirector.generated.h"

UCLASS()
class SWARM_API ABaseAIDirector : public AActor
{
	GENERATED_BODY()

		// Functions
public:

	ABaseAIDirector();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void GetSpawnPoints();
	void SpawnMonster(int32 WaveNumber);
	int32 WaveEnd(int32 WaveNumber, int32 MonsterPoints);

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void SpawnMonsterInWorld(FVector Location, FRotator Rotation, FVector Scale, EMonsterType Type);

	// Members
public:
	float m_fTimer;
	float m_fInterval;

	UPROPERTY(BlueprintReadWrite, Category = "Stats") int32 MinWaveSpitter;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") int32 MinWaveSlammer;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") int32 MinWaveBroodmother;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") int32 MonstersSpawned;

	UPROPERTY(BlueprintReadWrite, Category = "Stats") TArray<EMonsterType> Monsters;

	float SwarmerSpawnRatio;
	float SpitterSpawnRatio;
	float SlammerSpawnRatio;
	float BroodmotherSpawnRatio;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats") int32 MonsterLimit;

private:
	TArray<ABaseSpawnPoint*> m_SpawnPointsRegular;
	TArray<ABaseSpawnPoint*> m_SpawnPointsBroodmother;
};
