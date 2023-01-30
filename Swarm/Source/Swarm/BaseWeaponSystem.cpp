// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseWeaponSystem.h"
#include "Swarm.h"
#include "BaseProjectile.h"
#include "SwarmCharacter.h"

// Sets default values
ABaseWeaponSystem::ABaseWeaponSystem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	bAlwaysRelevant = true;

	CurrentWeapon = 1;
	CurrentWeight = 0;
	MaxWeight = 10;
}

// Called when the game starts or when spawned
void ABaseWeaponSystem::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABaseWeaponSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeaponSystem::ShootCall(bool ShootActive)
{
	ShootButtonActive = ShootActive;

	ResetMeleeArray();

	if (!ShootButtonActive || !ActiveWeapon)
	{
		return;
	}

	if (IsValid(ActiveWeapon))
	{
		if (!ActiveWeapon->IsMelee)
		{
			if (ActiveWeapon->IsReloading || ActiveWeapon->CurrentAmmo < 1)
			{
				return;
			}
		}

		if (ActiveWeapon->IsShooting)
		{
			return;
		}

		ActiveWeapon->IsShooting = true;

		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(Timer, this, &ABaseWeaponSystem::ShootLoopHandler, ActiveWeapon->ShootTime, false);

		if (!ActiveWeapon->IsMelee)
		{
			ActiveWeapon->FireBullet();
		}
	}
}

void ABaseWeaponSystem::ShootLoopHandler()
{
	if (ShootButtonActive && ActiveWeapon->CurrentAmmo > 0 && (ActiveWeapon->IsAutomatic))
	{
		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(Timer, this, &ABaseWeaponSystem::ShootLoopHandler, ActiveWeapon->ShootTime, false);

		ActiveWeapon->FireBullet();

		ActiveWeapon->IsShooting = true;
	}
	else
	{
		ActiveWeapon->IsShooting = false;
	}
}

void ABaseWeaponSystem::ReloadCall()
{
	if (IsValid(ActiveWeapon))
	{
		if (!ActiveWeapon->IsMelee)
		{
			ActiveWeapon->ReloadCall();
		}
	}
}

void ABaseWeaponSystem::SwitchWeapon(int32 WeaponNumber, bool SkipQS)
{
	if (ActiveWeapon)
	{
		if (ActiveWeapon->GetIsReloading() || ActiveWeapon->GetIsShooting())
		{
			return;
		}
	}

	if (Inventory.Num() < 1)
	{
		return;
	}

	if ((WeaponNumber < Inventory.Num()) && (WeaponNumber >= 0))
	{
		if (Inventory[WeaponNumber] != nullptr)
		{
			if (!SkipQS)
			{
				for (int i = 0; i < Inventory.Num(); ++i)
				{
					if (ActiveWeapon == Inventory[i])
					{
						LastWeapon = i;
						break;
					}
				}

				if (ActiveWeapon)
				{
					// Detach from parent
					ActiveWeapon->DetachRootComponentFromParent();
					ActiveWeapon->SetActorHiddenInGame(true);

					if (ActiveWeapon->Projectile)
					{
						ActiveWeapon->Projectile->SetActorHiddenInGame(true);
					}
				}
			}

			ActiveWeapon = nullptr;
			ActiveWeapon = Inventory[WeaponNumber];

			// Attach new weapon
			ASwarmCharacter* pOwner = Cast<ASwarmCharacter>(this->GetOwner());

			if (IsValid(pOwner))
			{
				ActiveWeapon->AttachToComponent(pOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Gun");
				ActiveWeapon->SetActorRelativeScale3D(FVector(0.35f));
				ActiveWeapon->SetActorHiddenInGame(false);
				ActiveWeapon->DisableComponentsSimulatePhysics();
				ActiveWeapon->SetActorEnableCollision(false);

				if (ActiveWeapon->Projectile)
				{
					ActiveWeapon->Projectile->SetActorHiddenInGame(false);
				}

				pOwner->CurrentSlot = WeaponNumber;
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, "BaseWeaponSystem: Switch Weapon - Owner Invalid");
			}
		}
	}
}

void ABaseWeaponSystem::DropWeapon(FVector Velocity)
{
	if (!ActiveWeapon)
	{
		return;
	}

	if (ActiveWeapon->GetIsReloading() || ActiveWeapon->GetIsShooting())
	{
		return;
	}

	int iGunAmount = 0;

	for (int i = 0; i < Inventory.Num(); ++i)
	{
		ABaseGun* TempGun = Cast<ABaseGun>(Inventory[i]);

		if (TempGun != NULL)
		{
			iGunAmount++;
		}
	}

	if (iGunAmount <= 1)
	{
		return;
	}

	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if (Inventory[i] == ActiveWeapon)
		{
			Inventory[i] = nullptr;

			ActiveWeapon->DetachRootComponentFromParent();
			ActiveWeapon->SetActorEnableCollision(true);
			ActiveWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ActiveWeapon->WeaponMesh->SetSimulatePhysics(true);

			FVector Force;
			Force = Cast<ASwarmCharacter>(GetOwner())->GetFollowCamera()->GetForwardVector() * (99999 / 32);
			Force += (Velocity);

			ActiveWeapon->WeaponMesh->AddForce(Force);
			//ActiveWeapon->SetOwner(NULL);
			CurrentWeight -= ActiveWeapon->Weight;
			ActiveWeapon = nullptr;
			SwitchWeapon(LastWeapon, true);

			return;
		}
	}
}

bool ABaseWeaponSystem::PickupWeapon()
{
	bool bResult = false;

	if (ActiveWeapon)
	{
		if (ActiveWeapon->GetIsReloading() || ActiveWeapon->GetIsShooting())
		{
			return bResult;
		}
	}

	FHitResult HitResult;

	if (Cast<ASwarmCharacter>(GetOwner())->CharacterInteractCheck(ECC_WorldStatic, HitResult, 600.0f))
	{
		ABaseGun* Gun = Cast<ABaseGun>(HitResult.Actor.Get());

		if (IsValid(Gun))
		{
			PickupHit(Gun);
			bResult = true;
		}
	}

	return bResult;
}

void ABaseWeaponSystem::PickupHit(ABaseGun* HitGun)
{
	HitGun->SetOwner(GetOwner());

	bool bNoExistingSlot = true;

	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if (Inventory[i] == nullptr)
		{
			Inventory[i] = HitGun;
			SwitchWeapon(i, false);
			CurrentWeight += HitGun->Weight;
			bNoExistingSlot = false;
			break;
		}
	}

	if (bNoExistingSlot)
	{
		Inventory.Add(HitGun);
		SwitchWeapon(Inventory.Num(), false);
	}

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &ABaseWeaponSystem::FixOverfillIntentory, 0.1f, false);
}

void ABaseWeaponSystem::FixOverfillIntentory()
{
	if (CurrentWeight > MaxWeight)
	{
		DropWeapon(GetOwner()->GetVelocity());
		Cast<ASwarmCharacter>(this->GetOwner())->UpdateAnimationPose();
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::FromInt(CurrentWeight));
	}
}

void ABaseWeaponSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseWeaponSystem, ActiveWeapon);
	DOREPLIFETIME(ABaseWeaponSystem, CurrentWeapon);
	DOREPLIFETIME(ABaseWeaponSystem, Inventory);
	DOREPLIFETIME(ABaseWeaponSystem, CurrentWeight);
}

bool ABaseWeaponSystem::InventoryFull(int32 QueryWeight)
{
	int iTempWeight = CurrentWeight + QueryWeight;

	if (iTempWeight > MaxWeight)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int32 ABaseWeaponSystem::AddWeaponToInventory(ABaseGun* Weapon)
{
	// Can assume Alex has done checking and weapon will ALWAYS be able to be added
	Weapon->SetOwner(this->GetOwner());

	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if (!IsValid(Inventory[i]))
		{
			Inventory[i] = Weapon;
			SwitchWeapon(i + 1, false);
			return i;
		}
	}

	Inventory.Add(Weapon);
	SwitchWeapon(Inventory.Num(), false);

	return Inventory.Num();
}

bool ABaseWeaponSystem::MoveWeapon(int32 OldSlot, int32 NewSlot)
{
	if (Inventory[OldSlot] == nullptr)
	{
		return false;
	}

	ABaseGun* TempGun = Inventory[OldSlot];

	if (Inventory[NewSlot] == nullptr)
	{
		Inventory[OldSlot] = nullptr;
	}
	else
	{
		Inventory[OldSlot] = Inventory[NewSlot];
	}
	
	Inventory[NewSlot] = TempGun;

	SwitchWeapon(NewSlot, false);
	ASwarmCharacter* TempChar = Cast<ASwarmCharacter>(this->GetOwner());

	TempChar->UpdateAnimationPose();
	TempChar->CurrentSlot = NewSlot;

	return true;
}