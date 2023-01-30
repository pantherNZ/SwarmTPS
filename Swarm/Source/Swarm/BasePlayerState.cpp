// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlayerState.h"
#include "Swarm.h"

ABasePlayerState::ABasePlayerState()
{
	Health = 100.0f;
	Stamina = 100.0f;
	Kills = 0;
	Money = 0;
	NameMaxWidth = 15;
	SetPlayerName("Unnamed");
}

ABasePlayerState::~ABasePlayerState()
{

}

void ABasePlayerState::UpdateHealth(float Amount)
{
	Health += Amount;
	Health = FMath::Max(0.0f, FMath::Min(100.0f, Health));
}

void ABasePlayerState::SetGamePlayerName(FString Name)
{
	SetPlayerName(Name);
}

bool ABasePlayerState::IsValidPlayerName(FString Name)
{
	return(Name.Len() >= 3 && Name.Len() <= NameMaxWidth);
}

void ABasePlayerState::WaveEndClient_Implementation()
{
	WaveEnd();
}

void ABasePlayerState::UpdateMoney_Implementation(float Val, FVector Location)
{
	Money += Val;

	if (Val > 0)
	{
		MoneyEarned += Val;
		ShowMoneyOnHUD(Val, Location);
	}
}

bool ABasePlayerState::UpdateMoney_Validate(float Val, FVector Location)
{
	return(true);
}

void ABasePlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerState, Health);
	DOREPLIFETIME(ABasePlayerState, Money);
	DOREPLIFETIME(ABasePlayerState, Kills);
	DOREPLIFETIME(ABasePlayerState, Ready);
	DOREPLIFETIME(ABasePlayerState, DamageDealt);
	DOREPLIFETIME(ABasePlayerState, ShotsFired);
	DOREPLIFETIME(ABasePlayerState, ShotsHit);
	DOREPLIFETIME(ABasePlayerState, MoneyEarned);
	DOREPLIFETIME(ABasePlayerState, MoneySpent);
	DOREPLIFETIME(ABasePlayerState, Stamina);
}