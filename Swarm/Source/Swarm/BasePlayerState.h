// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "BasePlayerState.generated.h"

/**
*
*/
UCLASS()
class SWARM_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()

		// Functions
public:
	ABasePlayerState();
	~ABasePlayerState();

	UFUNCTION(BlueprintCallable, Category = "Stats") 
	void UpdateHealth(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats") 
	void SetGamePlayerName(FString Name);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool IsValidPlayerName(FString Name);

	UFUNCTION(BlueprintCallable, Category = "Stats", Client, Reliable) 
	void WaveEndClient();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats") 
	void WaveEnd();

	UFUNCTION(BlueprintCallable, Category = "Stats", Server, Reliable, WithValidation) 
	void UpdateMoney(float Val, FVector Location);
	void UpdateMoney_Implementation(float Val, FVector Location);
	bool UpdateMoney_Validate(float Val, FVector Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void ShowMoneyOnHUD(float Val, FVector Location);

	// Members
public:

	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) float Health;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) float Stamina;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 Money;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 Kills;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) bool Ready;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 DamageDealt;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 ShotsFired;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 ShotsHit;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 MoneyEarned;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 MoneySpent;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") int32 NameMaxWidth;
};
