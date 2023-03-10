// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UBPSaveLoadGraphicsSettings.generated.h"

/**
 * 
 */
UCLASS()
class SWARM_API UUBPSaveLoadGraphicsSettings : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static UGameUserSettings* GetGameUserSettings();

	UFUNCTION(BlueprintCallable, Category = "Video Settings")
	static bool GetSupportedScreenResolutions(TArray<FString>& Resolutions);

	UFUNCTION(BlueprintPure, Category = "Video Settings")
	static FString GetScreenResolution();

	UFUNCTION(BlueprintPure, Category = "Video Settings")
	static int32 GetWindowMode();

	UFUNCTION(BlueprintCallable, Category = "Video Settings")
	static bool SetScreenResolution(const int32 Width, const int32 Height, const int32 Mode);

	UFUNCTION(BlueprintCallable, Category = "Video Settings")
	static bool ChangeScreenResolution(const int32 Width, const int32 Height, const int32 Mode);

	UFUNCTION(BlueprintCallable, Category = "Video Settings")
	static bool GetVideoQualitySettings(int32& AntiAliasing, int32& Effects, int32& PostProcess,
										int32& Resolution, int32& Shadow, int32& Texture, int32& ViewDistance);

	UFUNCTION(BlueprintCallable, Category = "Video Settings")
	static bool SetVideoQualitySettings(const int32 AntiAliasing, const int32 Effects, const int32 PostProcess,
										const int32 Resolution, const int32 Shadow, const int32 Texture, const int32 ViewDistance);

	UFUNCTION(BlueprintPure, Category = "Video Settings")
	static bool IsVSyncEnabled();

	UFUNCTION(BlueprintCallable, Category = "Video Settings")
	static bool SetVSyncEnabled(const bool VSync);

	UFUNCTION(BlueprintCallable, Category = "Video Settings")
	static bool SaveVideoModeAndQuality();

	UFUNCTION(BlueprintCallable, Category = "Video Settings")
	static bool RevertVideoMode();
};
