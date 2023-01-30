// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "GameFramework/Character.h"
#include "Utility.h"
#include "Net/UnrealNetwork.h"
#include "BaseMonster.generated.h"

UCLASS()
class SWARM_API ABaseMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseMonster(const class FObjectInitializer& PCIP);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Functions
	float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void DealDamage(bool LaunchPlayer);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	AActor* GetCurrentTarget(bool& Success);

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void GiveMoneyToPlayers(float Money, FVector Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "Decal")
	void SpawnDecal(FVector Direction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Initialise")
	void Initalise();

	UFUNCTION(Reliable, NetMulticast, WithValidation)
	void Death();
	virtual void Death_Implementation();
	virtual bool Death_Validate();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void ShowMonsterOutline();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void CheckOutlineTimer();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RotateToMovement();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RotateToTarget();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void IncreaseKillCount();

	// Common monster variables
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) EMonsterState State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) EMonsterState State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float AttackDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")	EMonsterType MonsterType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) UParticleSystem* HitParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") int32 KillReward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")	AActor* Target;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true")) UBoxComponent* Collision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float Leadership;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") TArray<ABaseMonster*> Followers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") bool CanFollow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") FVector DeathPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float DeathStrength;

	// Gibs classes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") UClass* GibBlueprint;
};
