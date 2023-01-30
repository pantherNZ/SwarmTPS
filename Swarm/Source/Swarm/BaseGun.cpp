// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseGun.h"
#include "Swarm.h"
#include "BaseProjectile.h"
#include "SwarmCharacter.h"
#include "BasePlayerState.h"
#include "BaseGameState.h"
#include "BaseMonster.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABaseGun::ABaseGun(const class FObjectInitializer& PCIP)
{
	IsShooting = false;
	IsReloading = false;
	CurrentAmmo = 0;
	MaxClipAmmo = 0;
	TotalAmmo = 0;
	IsAutomatic = true;
	IsMelee = false;

	WeaponMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, "Weapon Mesh");
	WeaponMesh->SetMobility(EComponentMobility::Movable);
	WeaponMesh->bReceivesDecals = false;
	WeaponMesh->SetUsingAbsoluteLocation(false);
	WeaponMesh->SetUsingAbsoluteRotation(false);
	WeaponMesh->SetUsingAbsoluteScale(false);
	WeaponMesh->bCastDynamicShadow = true;
	WeaponMesh->bOnlyOwnerSee = false;
	WeaponMesh->bOwnerNoSee = false;
	WeaponMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetComponentTickEnabled(true);
	RootComponent = WeaponMesh;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicatingMovement(true);
	bAlwaysRelevant = true;

	// Setup base stats
	WeaponStats.Accuracy = 0.2f;
	WeaponStats.Damage = 0.0f;
	WeaponStats.FireRate = 1.0f;
	WeaponStats.Lifesteal = 0.0f;

	FWeaponStats MinStats;
	MinStats.FireRate = 1.0f;
	MinStats.Accuracy = 0.2f;
	MinStats.Lifesteal = 0.0f;
	FWeaponStats MaxStats;
	MaxStats.FireRate = 2.0f;
	MaxStats.Accuracy = 1.0f;
	WeaponStatsMinMax.Add(MinStats);
	WeaponStatsMinMax.Add(MaxStats);
}

// Called when the game starts or when spawned
void ABaseGun::BeginPlay()
{
	Super::BeginPlay();

	WeaponStats.MagazineSize = this->MaxClipAmmo;
	WeaponStatsMinMax[0].MagazineSize = this->MaxClipAmmo;
	WeaponStatsMinMax[1].MagazineSize = this->MaxClipAmmo * 2;
}

// Called every frame
void ABaseGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(this->GetOwner()))
	{
		Destroy();
	}
}

void ABaseGun::FireBullet()
{
	ABaseGameState* pGameState = Cast<ABaseGameState>(GetWorld()->GetGameState());

	if (!IsValid(pGameState))
	{
		return;
	}

	if (UseProjectile)
	{
		if (GetWorld())
		{
			//FActorSpawnParameters SpawnParams;
			//SpawnParams.Owner = this;
			//SpawnParams.Instigator = Cast<ASwarmCharacter>(this->GetOwner());

			Projectile = Cast<ABaseProjectile>(GetWorld()->SpawnActor(ProjectileClass));

			if (Projectile)
			{
				if (!pGameState->InLobby())
				{
					CurrentAmmo -= 1;
				}

				FVector TempVelocityVector = Cast<ASwarmCharacter>(this->GetOwner())->GetFollowCamera()->GetForwardVector();
				TempVelocityVector.Normalize();

				TempVelocityVector *= 4000.0f;
				TempVelocityVector.Z += 250.0f;

				Projectile->SetActorLocation(WeaponMesh->GetSocketLocation("Muzzle"));

				//Projectile->Movement->SetVelocityInLocalSpace(TempVelocityVector);
				//Projectile->Movement->ProjectileGravityScale = ProjectileGravityScale;
				//Projectile->Movement->MaxSpeed = ProjectileSpeed;
				//Projectile->Activated = true;

				Projectile->ApplyVelocity(TempVelocityVector, true, ProjectileGravityScale, ProjectileSpeed);
				Projectile->SetOwner(this->GetOwner());

				Projectile->Gun = this;
			}
		}
	}
	else
	{
		if (!pGameState->InLobby())
		{
			CurrentAmmo -= 1;
			Cast<ABasePlayerState>(Cast<ASwarmCharacter>(this->GetOwner())->GetPlayerState())->ShotsFired++;
		}

		FHitResult HitResult = ShootHandler();
		ShootHitResult = HitResult;

		if (!HitResult.bBlockingHit)
		{
			return;
		}

		AActor* pHitActor;
		pHitActor = HitResult.Actor.Get(true);

		if (pHitActor->GetDebugName(pHitActor).Find("Enemy") != -1)
		{
			if (HitResult.BoneName.ToString().Contains("head"))
			{
				// Apply extra damage to hit actor for headshot
				UGameplayStatics::ApplyDamage(pHitActor, (WeaponStats.Damage * 3), 0, this, 0);
				PlayImpactSound(HitResult.ImpactPoint);
			}
			else
			{
				// Apply Damage to hit actor
				UGameplayStatics::ApplyDamage(pHitActor, WeaponStats.Damage, 0, this, 0);
			}

			for (int i = 2; i <= 3; ++i)
			{
				if (Cast<ABaseMonster>(pHitActor)->HitParticle == ParticleTemplates[i])
				{
					MulticastImpact(HitResult.ImpactPoint, i);
					break;
				}
			}
		}
		else if (pHitActor->GetDebugName(pHitActor).Find("Player") != -1)
		{
			// Apply Damage to hit actor
			UGameplayStatics::ApplyDamage(pHitActor, (WeaponStats.Damage / 10.0f), 0, this, 0);
			MulticastImpact(HitResult.ImpactPoint, 1);
		}
		else
		{
			// Hit ground
			MulticastImpact(HitResult.ImpactPoint, 0);
		}
	}
}

FHitResult ABaseGun::ShootHandler()
{
	FVector StartPos;
	FVector EndPos;

	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);

	if (IsValid(this->GetOwner()))
	{
		UCameraComponent* pCamera = Cast<ASwarmCharacter>(this->GetOwner())->GetFollowCamera();
		StartPos = pCamera->GetComponentLocation();
		//EndPos = OwningPlayer->GetFollowCamera()->GetForwardVector() * 999999.0f;

		float fAccuracyInv = 1.0f - WeaponStats.Accuracy;

		FRotator Rot((FMath::FRandRange(-fAccuracyInv, fAccuracyInv)), (FMath::FRandRange(-fAccuracyInv, fAccuracyInv)), 0.0f);
		FVector Forward = Rot.RotateVector(pCamera->GetForwardVector());
		EndPos = StartPos + Forward * 9999999.0f;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Owning Player not valid");
		return RV_Hit;
	}

	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = true;
	//RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;

	ECollisionChannel ObjectTypes[2];
	ObjectTypes[1] = ECC_Visibility;

	//call GetWorld() from within an actor extending class
	GetWorld()->LineTraceSingleByChannel(
		RV_Hit,        //result
		StartPos,    //start
		EndPos, //end
		//ObjectTypes[2], //collision channel
		ECC_Visibility,
		RV_TraceParams
		);

	//DrawDebugLine(GetWorld(), StartPos, RV_Hit.Location, FColor(255, 0, 0), true, -1, 0, 3);

	return RV_Hit;
}

void ABaseGun::ReloadCall()
{
	// Check to see if we can reload
	if ((CurrentAmmo != MaxClipAmmo) && (TotalAmmo > 0) && (!IsShooting))
	{
		// Checks passed, we can start the reload now
		StartReload();
	}
}

void ABaseGun::StartReload()
{
	if (!(IsShooting && IsReloading))
	{
		FTimerHandle Timer;
		IsReloading = true;

		GetWorld()->GetTimerManager().SetTimer(Timer, this, &ABaseGun::FinishReload, ReloadTime, false);
	}
}

void ABaseGun::FinishReload()
{
	// Time to end the reloading
	IsReloading = false;

	float fTempCurrent = 0.0f;

	if (TotalAmmo >= MaxClipAmmo)
	{
		fTempCurrent = MaxClipAmmo;
		TotalAmmo = TotalAmmo - (MaxClipAmmo - CurrentAmmo);
	}
	else
	{
		fTempCurrent = TotalAmmo;
		TotalAmmo = TotalAmmo - fTempCurrent;
	}

	CurrentAmmo = fTempCurrent;
}

bool ABaseGun::GetIsShooting()
{
	return IsShooting;
}

bool ABaseGun::GetIsReloading()
{
	return IsReloading;
}

int32 ABaseGun::GetCurrentUpgradePercent(int32 Stat)
{
	switch (Stat)
	{
	case(0) :
	{
		float fIncreased = (WeaponStats.Damage - WeaponStatsMinMax[0].Damage);
		float fGap = (WeaponStatsMinMax[1].Damage - WeaponStatsMinMax[0].Damage);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	case(1) :
	{
		float fIncreased = (WeaponStats.Accuracy - WeaponStatsMinMax[0].Accuracy);
		float fGap = (WeaponStatsMinMax[1].Accuracy - WeaponStatsMinMax[0].Accuracy);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	case(2) :
	{
		float fIncreased = (WeaponStats.FireRate - WeaponStatsMinMax[0].FireRate);
		float fGap = (WeaponStatsMinMax[1].FireRate - WeaponStatsMinMax[0].FireRate);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	case(3) :
	{
		float fIncreased = (this->MaxClipAmmo - WeaponStatsMinMax[0].MagazineSize);
		float fGap = (WeaponStatsMinMax[1].MagazineSize - WeaponStatsMinMax[0].MagazineSize);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	case(4) :
	{
		float fIncreased = (WeaponStats.Lifesteal - WeaponStatsMinMax[0].Lifesteal);
		float fGap = (WeaponStatsMinMax[1].Lifesteal - WeaponStatsMinMax[0].Lifesteal);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	};

	return(0);
}

float ABaseGun::GetUpgradeStepValue(int32 Stat)
{
	switch (Stat)
	{
	case(0) :
		return((WeaponStatsMinMax[1].Damage - WeaponStatsMinMax[0].Damage) / 10.0f);
	case(1) :
		return((WeaponStatsMinMax[1].Accuracy - WeaponStatsMinMax[0].Accuracy) / 10.0f);
	case(2) :
		return((WeaponStatsMinMax[1].FireRate - WeaponStatsMinMax[0].FireRate) / 10.0f);
	case(3) :
	{
		switch (this->GunType)
		{
		case(EWeaponType::EGT_PISTOL) :
			return(4);
		case(EWeaponType::EGT_GRENADELAUNCHER) :
			return(1);
		case(EWeaponType::EGT_RIFLE) :
			return(6);
		};
	}
	case(4) :
		return((WeaponStatsMinMax[1].Lifesteal - WeaponStatsMinMax[0].Lifesteal) / 10.0f);
	};

	return(0.0f);
}

void ABaseGun::AddUpgrade(int32 Stat)
{
	switch (Stat)
	{
	case(0) :
	{
		WeaponStats.Damage += this->GetUpgradeStepValue(0);
		break;
	}
	case(1) :
	{
		WeaponStats.Accuracy += this->GetUpgradeStepValue(1);
		break;
	}
	case(2) :
	{
		float fChange = this->GetUpgradeStepValue(2);
		WeaponStats.FireRate += fChange;

		ShootTime = BaseShootTime - (BaseShootTime * (this->GetCurrentUpgradePercent(2) / 100.0f));
		break;
	}
	case(3) :
	{
		this->MaxClipAmmo += GetUpgradeStepValue(3);
		break;
	}
	case(4) :
	{
		WeaponStats.Lifesteal += this->GetUpgradeStepValue(4);
		break;
	}
	};
}

void ABaseGun::MulticastImpact_Implementation(FVector _Impact, int32 ImpactID)
{
	// Spawn particle for impact
	if (ParticleTemplates.Num() >= (ImpactID + 1))
	{
		(UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleTemplates[ImpactID], _Impact))->SetWorldScale3D(FVector(3.0f, 3.0f, 3.0f));
	}
}

void ABaseGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseGun, IsShooting);
	DOREPLIFETIME(ABaseGun, IsReloading);
	DOREPLIFETIME(ABaseGun, TotalAmmo);
	DOREPLIFETIME(ABaseGun, MaxClipAmmo);
	DOREPLIFETIME(ABaseGun, CurrentAmmo);
	DOREPLIFETIME(ABaseGun, ProjectileClass);
	DOREPLIFETIME(ABaseGun, GunType);
}