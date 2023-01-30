// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "Utility.h"
#include "BaseAIDirector.h"
#include "BaseGameState.generated.h"
/**
*
*/
UCLASS()
class SWARM_API ABaseGameState : public AGameState
{
	GENERATED_BODY()

public:
	// Functions
	ABaseGameState();

	void GetAIDirector();
	virtual void Tick(float DeltaTime);
	virtual void BeginPlay();
	void CheckGameOver();
	bool CheckPlayersReady();

	void UpdateActualMonsterCount();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool InLobby();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void GameStart();

	// Members
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) int32 WaveNumber;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) int32 PlayerReadyCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) int32 CurrentWaveKills;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) int32 MonstersMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) int32 MonsterPoints;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) int32 TotalKills;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) int32 MonstersAlive;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) float WaveTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) EGameState GameState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) bool InStartLobby;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") ABaseAIDirector* AIDirector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float WaveIntervalLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float HealthLostDuringWave;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float WaveTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float Difficulty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") UClass* CharacterClass;
	FTimerHandle CheckMonsterCountTimer;
	int32 Decreased;
};
