// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Character.h"
#include "SwarmCharacter.generated.h"

UCLASS(config=Game)
class ASwarmCharacter : public ACharacter
{
	GENERATED_BODY()

	// Camera boom
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) class USpringArmComponent* CameraBoom;

	// Cameras
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) class UCameraComponent* DeathCamera;

public:
	ASwarmCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera) float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera) float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data") float Sensitivity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data") float Pitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data") float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data") float Direction;

	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 Lives;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) float ReviveProgress;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) bool IsSprinting;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) ASwarmCharacter* RevivingCharacter;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) int32 CurrentSlot;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") bool BeingRevived;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") bool Reviving;

	// Speed of decaying (Decay * Delta (revive is from 0 - 1))
	UPROPERTY(BlueprintReadWrite, Category = "Stats") float DecaySpeed;
	// Speed of reviving (Decay * Delta (revive is from 0 - 1))
	UPROPERTY(BlueprintReadWrite, Category = "Stats") float ReviveSpeed;

	// Camera zoom data
	UPROPERTY(BlueprintReadWrite, Category = "Stats") bool CameraZoomToggle;
	float CameraZoomScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float CameraZoomMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float CameraZoomMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float CameraZoomSpeed;

	// Death camera variables
	UPROPERTY(BlueprintReadWrite, Category = "Stats") float DeathCameraPitch;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") float DeathCameraYaw;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") float CameraBlend;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") bool CameraBlendToDeath;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") FVector TargetLocation;

	// Jumping
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) bool IsJumping;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") bool CanMove;

	// Movement speeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float RunSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float SprintSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") float BatteryRunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") TArray<UParticleSystemComponent*> PersistantParticleList;

	// Used for grenade launcher statistics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") int32 GrenadeID; 

protected:

	void MoveForward(float Value);
	void MoveRight(float Value);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void ConfirmKill(int32 KillReward, FVector Location);
	void ConfirmHit(int32 Damage);
	void ConfirmHit(int32 Damage, UINT32 MonsterID);

	void RemoveGrenadeID();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void UpdateAnimationPose();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool ReviveCheck();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool CharacterInteractCheck(ECollisionChannel Channel, FHitResult& HitResult, float Distance);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool IsAlive();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void GameStarting();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Stats")
	void SwitchToRegularCamera();

	UFUNCTION(Reliable, BlueprintCallable, Client, Category = "Stats")
	void SwitchToRegularCameraClient();
	bool SwitchToRegularCameraClient_Validate();
	void SwitchToRegularCameraClient_Implementation();

	void JumpCheck();
	void YawRotation(float Val);
	void PitchRotation(float Val);

	UFUNCTION(BlueprintCallable, Server, Category = "Stats", Reliable, WithValidation)
	void ServerSetReviveRef(ASwarmCharacter* Character);
	void ServerSetReviveRef_Implementation(ASwarmCharacter* Character);
	bool ServerSetReviveRef_Validate(ASwarmCharacter* Character);

	UFUNCTION(BlueprintCallable, Server, Category = "Stats", Reliable, WithValidation)
	void JumpServer();
	void JumpServer_Implementation();
	bool JumpServer_Validate();

	UFUNCTION(BlueprintCallable, NetMulticast, Category = "Stats", Reliable)
	void JumpMulticast();
	void JumpMulticast_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void CameraHandler();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void CalculateDeathCameraPosition();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCameraRelativePosition();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void RevivePlayer();

	UFUNCTION(BlueprintCallable, NetMulticast, Category = "Stats", Reliable)
	void SpawnParticleMulticast(UParticleSystem* Particle, FVector Scale, FVector Position, int32 Index, FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f), USceneComponent* AttachTo = nullptr);
	void SpawnParticleMulticast_Implementation(UParticleSystem* Particle, FVector Scale, FVector Position, int32 Index, FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f), USceneComponent* AttachTo = nullptr);

	UFUNCTION(BlueprintCallable, NetMulticast, Category = "Stats", Reliable)
	void DeactivateParticleMulticast(int32 Index);
	void DeactivateParticleMulticast_Implementation(int32 Index);
};

