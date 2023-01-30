// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Swarm.h"
#include "BaseGun.h"
#include "GameFramework/Actor.h"
#include "BaseWeaponSystem.generated.h"

UCLASS()
class SWARM_API ABaseWeaponSystem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseWeaponSystem();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "System Functions")
	void ShootCall(bool ShootActive);

	void ShootLoopHandler();

	UFUNCTION(BlueprintCallable, Category = "System Functions")
	void ReloadCall();

	UFUNCTION(BlueprintCallable, Category = "System Functions")
	void SwitchWeapon(int32 WeaponNumber, bool SkipQS);

	UFUNCTION(BlueprintCallable, Category = "System Functions")
	void DropWeapon(FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "System Functions")
	bool PickupWeapon();

	UFUNCTION(BlueprintCallable, Category = "System Functions")
	bool MoveWeapon(int32 OldSlot, int32 NewSlot);

	UFUNCTION(BlueprintCallable, Category = "System Functions")
	bool InventoryFull(int32 QueryWeight);

	UFUNCTION(BlueprintCallable, Category = "System Functions")
	int32 AddWeaponToInventory(ABaseGun* Weapon);

	void PickupHit(ABaseGun* HitGun);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	ABaseGun* ActiveWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<ABaseGun*> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LastWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	int32 CurrentWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated) int32 CurrentWeapon;

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void ResetMeleeArray();

	void FixOverfillIntentory();

	bool ShootButtonActive;
};
