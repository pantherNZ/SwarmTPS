// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utility.h"
#include "Swarm.h"
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "BaseSessionhandler.generated.h"

UCLASS()
class SWARM_API ABaseSessionhandler : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseSessionhandler(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Network")
	void Start(FName SessionName, bool bIsLAN, int32 MaxNumPlayers);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void UpdateServers();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void GetServerDetails(int32 Server, FString& ServerName, int32& Ping, int32& CurrentPlayers, int32& MaxPlayers);

	UFUNCTION(BlueprintCallable, Category = "Network")
	int32 SessionResultsFound();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinOnlineGame(int32 ServerID);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void DestroySession();

	bool HostSession(TSharedPtr<FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);
	void FindSessions(TSharedPtr<FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence);
	void OnFindSessionsComplete(bool bWasSuccessful);
	bool JoinSession(TSharedPtr<FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;

	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	bool bSearch;
	bool bCurrentlySearching;

	//FName SessionName;
};
