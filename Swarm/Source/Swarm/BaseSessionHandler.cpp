// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseSessionhandler.h"
#include "Swarm.h"
#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "Online.h"
#include "OnlineSubsystem.h"

// Sets default values
ABaseSessionhandler::ABaseSessionhandler(const FObjectInitializer& ObjectInitializer)
{
	SessionSearch = nullptr;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ABaseSessionhandler::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &ABaseSessionhandler::OnStartOnlineGameComplete);
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ABaseSessionhandler::OnFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &ABaseSessionhandler::OnJoinSessionComplete);
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &ABaseSessionhandler::OnDestroySessionComplete);

	bSearch = false;
	bCurrentlySearching = false;
}

// Called when the game starts or when spawned
void ABaseSessionhandler::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABaseSessionhandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ABaseSessionhandler::HostSession(TSharedPtr<FUniqueNetId> UserId, FName SessionName, bool bIsLan, bool bIsPresence, int32 MaxNumPlayers)
{
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())// && UserId.IsValid())
		{
			SessionSettings = MakeShareable(new FOnlineSessionSettings());

			SessionSettings->bIsLANMatch = true; // Hardcoded to avoid errors (Paramater is ignored)
			SessionSettings->bUsesPresence = true;
			SessionSettings->NumPublicConnections = MaxNumPlayers;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bAllowJoinInProgress = true; // Update this
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
			SessionSettings->bIsDedicated = false;

			FOnlineSessionSetting TempSetting;

			FVariantData ServerName;

			//ServerName.GetType()
			FString Temp = SessionName.ToString();
			ServerName.SetValue(Temp);

			TempSetting.Data = ServerName;

			SessionSettings->Settings.Add("Name", TempSetting);
			SessionSettings->Set(SETTING_MAPNAME, FString("FlatDarkMapV1"), EOnlineDataAdvertisementType::ViaOnlineService);

			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

			//return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
			return Sessions->CreateSession(NULL, SessionName, *SessionSettings);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("No OnlineSubsystem found!"));
	}

	return false;
}

void ABaseSessionhandler::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

			if (bWasSuccessful)
			{
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				Sessions->StartSession(SessionName);
			}
		}
	}
}

void ABaseSessionhandler::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), "FlatDarkMapV1", true, "listen");
	}
}

void ABaseSessionhandler::FindSessions(TSharedPtr<FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())// && UserId.IsValid())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());

			SessionSearch->bIsLanQuery = true; // Again, hardcoded to avoid issues parameter ignored
			SessionSearch->MaxSearchResults = 10;
			SessionSearch->PingBucketSize = 50;

			if (bIsPresence)
			{
				SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
			}

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
			//Sessions->FindSessions(*UserId, SearchSettingsRef);

			Sessions->FindSessions(NULL, SearchSettingsRef);

			bCurrentlySearching = true;
		}
	}
	else
	{
		OnFindSessionsComplete(false);
	}
}

void ABaseSessionhandler::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Session Search Finished");

	bCurrentlySearching = false;

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			if (SessionSearch->SearchResults.Num() > 0)
			{
				bSearch = true;

				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Sessions Found");

				////SessionSearch->SearchResults[0].Session.SessionSettings.Settings
				//
				//for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
				//{
				//	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx + 1, *(SessionSearch->SearchResults[SearchIdx].Session.OwningUserName)));
				//}
			}
		}
	}
}

bool ABaseSessionhandler::JoinSession(TSharedPtr<FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	bool bSuccessful = false;

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())// && UserId.IsValid())
		{
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			//bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
			bSuccessful = Sessions->JoinSession(NULL, SessionName, SearchResult);
		}
	}

	return bSuccessful;
}

void ABaseSessionhandler::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

			APlayerController* const PlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());

			FString TravelURL;

			if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
			{
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void ABaseSessionhandler::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		return;
	}

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		}
	}
}

void ABaseSessionhandler::Start(FName SessionName, bool bIsLAN, int32 MaxNumPlayers)
{
	ULocalPlayer* const Player = GEngine->GetFirstGamePlayer(GetWorld());
	
	HostSession(NULL, SessionName, bIsLAN, true, MaxNumPlayers);
	//HostSession(Player->GetPreferredUniqueNetId(), SessionName, bIsLAN, true, MaxNumPlayers);
}

void ABaseSessionhandler::UpdateServers()
{
	if (!bCurrentlySearching)
	{
		ULocalPlayer* const Player = GEngine->GetFirstGamePlayer(GetWorld());
		FindSessions(NULL, "DefaultName", true, false);
	//	FindSessions(Player->GetPreferredUniqueNetId(), "DefaultName", true, false);
	}
}

void ABaseSessionhandler::GetServerDetails(int32 Server, FString& ServerName, int32& Ping, int32& CurrentPlayers, int32& MaxPlayers)
{
	Ping = SessionSearch->SearchResults[Server].PingInMs;
	MaxPlayers = SessionSearch->SearchResults[Server].Session.SessionSettings.NumPublicConnections;
	
	const FOnlineSessionSetting* TempSetting = SessionSearch->SearchResults[Server].Session.SessionSettings.Settings.Find("Name");

	FString NameServer = "null";

	if (TempSetting)
	{
		TempSetting->Data.GetValue(NameServer);
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(1, 10.0f, FColor::Green, "Tempsetting not valid");
	}

	CurrentPlayers = MaxPlayers - SessionSearch->SearchResults[Server].Session.NumOpenPublicConnections;
	ServerName = SessionSearch->SearchResults[Server].Session.OwningUserName;
}

int32 ABaseSessionhandler::SessionResultsFound()
{
	if (bSearch)
	{
		if (SessionSearch.IsValid())
		{
			return SessionSearch->SearchResults.Num();
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

void ABaseSessionhandler::JoinOnlineGame(int32 ServerID)
{
	ULocalPlayer* const Player = GEngine->GetFirstGamePlayer(GetWorld());

	FOnlineSessionSearchResult SearchResult;

	if (SessionSearch->SearchResults.Num() > ServerID)
	{
		SearchResult = SessionSearch->SearchResults[ServerID];

//		JoinSession(Player->GetPreferredUniqueNetId(), GameSessionName, SearchResult);
		JoinSession(NULL, GameSessionName, SearchResult);
	}
}

void ABaseSessionhandler::DestroySession()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			Sessions->DestroySession(GameSessionName);
		}
	}
}