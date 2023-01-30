// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SwarmCharacter.h"
#include "Swarm.h"
#include "BaseWeaponSystem.h"
#include "BasePlayerState.h"
#include "BaseGameState.h"
#include "BaseMonster.h"
#include "Kismet/KismetMathLibrary.h"

//////////////////////////////////////////////////////////////////////////
// ASwarmCharacter

ASwarmCharacter::ASwarmCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 2000.f;
	GetCharacterMovement()->AirControl = 1.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->TargetArmLength = 280.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Death camera
	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCamera"));
	DeathCamera->AttachTo(RootComponent);

	Pitch = 0.0f;
	Direction = 0.0f;
	Speed = 0.0f;

	ReviveProgress = 0.0f;
	RevivingCharacter = nullptr;
	BeingRevived = false;

	DecaySpeed = 0.05f;
	ReviveSpeed = 0.3f;

	CameraZoomToggle = false;
	CameraZoomScale = 1.0f;
	CameraZoomMin = 150.0f;
	CameraZoomMax = 280.0f;
	CameraZoomSpeed = 8.0f;

	Lives = 5;

	DeathCameraPitch = 0.0f;
	DeathCameraYaw = 0.0f;
	CameraBlend = 0.0f;
	CameraBlendToDeath = false;

	RunSpeed = 600.0f;
	SprintSpeed = 1000.0;
	BatteryRunSpeed = 450.0f;
	CanMove = true;
	Sensitivity = 1.0f;

	GrenadeID = -1;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASwarmCharacter::SetupPlayerInputComponent(UInputComponent* inputComponent)
{
	// Set up gameplay key bindings
	check(inputComponent);
	inputComponent->BindAction("Jump", IE_Pressed, this, &ASwarmCharacter::JumpCheck);
	inputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	inputComponent->BindAxis("MoveForward", this, &ASwarmCharacter::MoveForward);
	inputComponent->BindAxis("MoveRight", this, &ASwarmCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	inputComponent->BindAxis("Turn", this, &ASwarmCharacter::YawRotation); // ASwarmCharacter::YawRotation
	inputComponent->BindAxis("LookUp", this, &ASwarmCharacter::PitchRotation);

	//inputComponent->BindAxis("TurnRate", this, &ASwarmCharacter::TurnAtRate); // ASwarmCharacter::TurnAtRate
	//inputComponent->BindAxis("LookUpRate", this, &ASwarmCharacter::LookUpAtRate);
}

void ASwarmCharacter::JumpCheck()
{
	if (IsAlive() && !this->GetCharacterMovement()->IsFalling() && CanMove)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			JumpMulticast();
		}
		else
		{
			IsJumping = true;
			JumpServer();
		}

		Jump();
	}
}

void ASwarmCharacter::JumpServer_Implementation()
{
	IsJumping = true;
}

void ASwarmCharacter::JumpMulticast_Implementation()
{
	IsJumping = true;
}

bool ASwarmCharacter::JumpServer_Validate()
{
	return(true);
}

void ASwarmCharacter::YawRotation(float Val)
{
	if (FollowCamera->IsActive())
	{
		AddControllerYawInput(Val * Sensitivity);
	}
	else if (DeathCamera->IsActive())
	{
		DeathCameraYaw += Val * Sensitivity;
		CalculateDeathCameraPosition();
	}
}

void ASwarmCharacter::PitchRotation(float Val)
{
	if (FollowCamera->IsActive())
	{
		AddControllerPitchInput(Val * Sensitivity);
	}
	else if (DeathCamera->IsActive())
	{
		DeathCameraPitch += Val * Sensitivity;
		DeathCameraPitch = FMath::Clamp(DeathCameraPitch, -85.0f, -5.0f);
		CalculateDeathCameraPosition();
	}
}

void ASwarmCharacter::CalculateDeathCameraPosition()
{
	float fRadius = 400.0f;
	float fYaw = FMath::DegreesToRadians(DeathCameraYaw);
	float fPitch = FMath::DegreesToRadians(DeathCameraPitch);

	float fX = fRadius * FMath::Cos(fYaw) * FMath::Sin(fPitch);
	float fY = fRadius * FMath::Sin(fYaw) * FMath::Sin(fPitch);
	float fZ = fRadius * FMath::Cos(fPitch);
	TargetLocation = FVector(fX, fY, fZ);

	//FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, FVector(0.0f, 0.0f, 0.0f));
	//DeathCamera->SetRelativeRotation(Rotation);
	SetCameraRelativePosition();
}

void ASwarmCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASwarmCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASwarmCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && IsAlive() && CanMove)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector DirectionVar = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(DirectionVar, Value);
	}
}

void ASwarmCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && IsAlive() && CanMove)
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector DirectionVar = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(DirectionVar, Value);
	}
}

void ASwarmCharacter::ConfirmKill(int32 KillReward, FVector Location) //_Implementation
{
	ABasePlayerState* pPlayerState = Cast<ABasePlayerState>(GetController()->PlayerState);

	if (IsValid(pPlayerState))
	{
		pPlayerState->UpdateMoney(KillReward, Location);
		pPlayerState->Kills++;
	}
}

void ASwarmCharacter::ConfirmHit(int32 Damage)
{
	ABasePlayerState* pPlayerState = Cast<ABasePlayerState>(GetController()->PlayerState);

	if (IsValid(pPlayerState))
	{
		pPlayerState->DamageDealt += Damage;
		pPlayerState->ShotsHit++;
	}
}

void ASwarmCharacter::ConfirmHit(int32 Damage, UINT32 NewGrenadeID)
{
	ABasePlayerState* pPlayerState = Cast<ABasePlayerState>(GetController()->PlayerState);

	if (IsValid(pPlayerState))
	{
		pPlayerState->DamageDealt += Damage;

		if (GrenadeID == -1)
		{
			GrenadeID = NewGrenadeID;
			pPlayerState->ShotsHit++;

			FTimerHandle Timer;
			GetWorld()->GetTimerManager().SetTimer(Timer, this, &ASwarmCharacter::RemoveGrenadeID, 0.05f, false);
		}
	}
}

void ASwarmCharacter::RemoveGrenadeID()
{
	GrenadeID = -1;
}

bool ASwarmCharacter::ReviveCheck()
{
	bool RevivingVar = false;

	ASwarmCharacter* ReviveCharTarg = nullptr;
	FHitResult HitResult;

	if (CharacterInteractCheck(ECC_Pawn, HitResult, 600.0f))
	{
		ASwarmCharacter* pCharacter = Cast<ASwarmCharacter>(HitResult.Actor.Get(true));

		if (IsValid(pCharacter) && Cast<ABasePlayerState>(pCharacter->GetPlayerState())->Health <= 0.0f && pCharacter->Lives > 0)
		{
			ReviveCharTarg = pCharacter;
			RevivingVar = true;
		}
		else
		{
			RevivingVar = false;
		}
	}

	// Tell server to update the reference
	ServerSetReviveRef(ReviveCharTarg);

	return(Reviving);
}

void ASwarmCharacter::ServerSetReviveRef_Implementation(ASwarmCharacter* Character)
{
	RevivingCharacter = Character;

	if (IsValid(RevivingCharacter)) BeingRevived = true;
	else BeingRevived = false;
}

bool ASwarmCharacter::ServerSetReviveRef_Validate(ASwarmCharacter* Character)
{
	return(true);
}

bool ASwarmCharacter::CharacterInteractCheck(ECollisionChannel Channel, FHitResult& HitResult, float Distance)
{
	FVector StartPos;
	StartPos = GetFollowCamera()->GetComponentLocation();

	FVector EndPos;
	EndPos = GetFollowCamera()->GetForwardVector() * Distance;

	EndPos += StartPos;

	bool bResult = VTraceSphere(this, StartPos, EndPos, 20.0f, HitResult, Channel);

	if (false)
	{
		DrawDebugLine(
			GetWorld(),
			StartPos,
			HitResult.Location,
			FColor(255, 0, 0),
			true, -1, 0,
			12.333
			);
	}

	return bResult;
}

bool ASwarmCharacter::IsAlive()
{
	ABasePlayerState* PS = Cast<ABasePlayerState>(GetPlayerState());

	if (IsValid(PS)) return(PS->Health > 0.0f);
	else return(false);
}

void ASwarmCharacter::CameraHandler()
{
	// Camera zoom blend
	if (CameraZoomToggle && CameraZoomScale > 0.0f)
	{
		CameraZoomScale -= GetWorld()->DeltaTimeSeconds * CameraZoomSpeed;
		CameraBoom->TargetArmLength = FMath::Lerp(CameraZoomMin, CameraZoomMax, CameraZoomScale);
		FollowCamera->FieldOfView = FMath::Lerp(70.0f, 90.0f, CameraZoomScale);
	}
	else if (!CameraZoomToggle && CameraZoomScale < 1.0f)
	{
		CameraZoomScale += GetWorld()->DeltaTimeSeconds * CameraZoomSpeed;
		CameraBoom->TargetArmLength = FMath::Lerp(CameraZoomMin, CameraZoomMax, CameraZoomScale);
		FollowCamera->FieldOfView = FMath::Lerp(70.0f, 90.0f, CameraZoomScale);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::SanitizeFloat(CameraBoom->RelativeRotation.Pitch));

	// Camera blend between follow & death
	if (CameraBlendToDeath && CameraBlend < 1.0f)
	{
		CameraBlend += GetWorld()->DeltaTimeSeconds;
		CameraBlend = FMath::Max(0.0f, FMath::Min(1.0f, CameraBlend));
		SetCameraRelativePosition();
	}
	else if (!CameraBlendToDeath && CameraBlend > 0.0f)
	{
		CameraBlend -= GetWorld()->DeltaTimeSeconds;
		CameraBlend = FMath::Max(0.0f, FMath::Min(1.0f, CameraBlend));
		SetCameraRelativePosition();
	}
}

void ASwarmCharacter::SetCameraRelativePosition()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::SanitizeFloat(CameraBlend));

	float fPitch = FMath::DegreesToRadians(CameraBoom->GetRelativeRotation().Pitch);
	FVector Offset = FVector(-CameraBoom->TargetArmLength * FMath::Cos(fPitch), 0.0f, -CameraBoom->TargetArmLength * FMath::Sin(fPitch));
	FVector FollowCameraPos = CameraBoom->GetRelativeLocation() + FollowCamera->GetRelativeLocation() + Offset;
	FVector FinalLocation = FMath::Lerp(FollowCameraPos, TargetLocation, CameraBlend);
	DeathCamera->SetRelativeLocation(FinalLocation);

	FVector TargetLookPos = FMath::Lerp(CameraBoom->GetRelativeLocation(), FVector(0.0f, 0.0f, 0.0f), CameraBlend);
	FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(FinalLocation, TargetLookPos);
	DeathCamera->SetRelativeRotation(Rotation);
}

void ASwarmCharacter::SwitchToRegularCameraClient_Implementation()
{
	SwitchToRegularCamera();
}

void ASwarmCharacter::SpawnParticleMulticast_Implementation(UParticleSystem* Particle, FVector Scale, FVector Position, int32 StoreIndex, FRotator Rotation, USceneComponent* AttachTo)
{
	UParticleSystemComponent* pCmp;

	if (IsValid(AttachTo))
	{
		pCmp = UGameplayStatics::SpawnEmitterAttached(Particle, AttachTo, NAME_None, Position, Rotation, EAttachLocation::SnapToTarget);
	}
	else
	{
		pCmp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, Position, Rotation);
	}

	pCmp->SetWorldScale3D(Scale);

	if (StoreIndex >= 0)
	{
		if (StoreIndex == PersistantParticleList.Num())
		{
			PersistantParticleList.Add(pCmp);
		}	
		else if (StoreIndex < PersistantParticleList.Num())
		{
			PersistantParticleList[StoreIndex] = pCmp;
		}
	}
}

void ASwarmCharacter::DeactivateParticleMulticast_Implementation(int32 Index)
{
	if (Index >= 0 && Index < PersistantParticleList.Num())
	{
		PersistantParticleList[Index]->DestroyComponent();
		PersistantParticleList.RemoveAt(Index);
	}
}

void ASwarmCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASwarmCharacter, ReviveProgress);
	DOREPLIFETIME(ASwarmCharacter, RevivingCharacter);
	DOREPLIFETIME(ASwarmCharacter, Lives);
	DOREPLIFETIME(ASwarmCharacter, CurrentSlot);
}