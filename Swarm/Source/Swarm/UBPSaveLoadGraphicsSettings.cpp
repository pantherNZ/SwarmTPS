// Fill out your copyright notice in the Description page of Project Settings.

#include "UBPSaveLoadGraphicsSettings.h"
#include "Swarm.h"

UGameUserSettings* UUBPSaveLoadGraphicsSettings::GetGameUserSettings()
{
	if (GEngine != nullptr)
	{
		return GEngine->GameUserSettings;
	}

	return nullptr;
}

bool UUBPSaveLoadGraphicsSettings::GetSupportedScreenResolutions(TArray<FString>& Resolutions)
{
	FScreenResolutionArray ResolutionsArray;

	if (RHIGetAvailableResolutions(ResolutionsArray, true))  // "RHI" dependency
	{
		for (const FScreenResolutionRHI& Resolution : ResolutionsArray)
		{
			FString StrW = FString::FromInt(Resolution.Width);
			FString StrH = FString::FromInt(Resolution.Height);
			Resolutions.AddUnique(StrW + "x" + StrH);
		}

		return true;
	}

	return false;  // failed to obtain screen resolutions
}

// Get currently set screen resolution
FString UUBPSaveLoadGraphicsSettings::GetScreenResolution()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return FString("");
	}

	FIntPoint Resolution = Settings->GetScreenResolution();
	return FString::FromInt(Resolution.X) + "x" + FString::FromInt(Resolution.Y);
}


// Check whether or not we are currently running in fullscreen mode
int32 UUBPSaveLoadGraphicsSettings::GetWindowMode()
{
	UGameUserSettings* Settings = GetGameUserSettings();

	return(static_cast<int>(Settings->GetFullscreenMode()));
}

// Set the desired screen resolution (does not change it yet)
bool UUBPSaveLoadGraphicsSettings::SetScreenResolution(
	const int32 Width, const int32 Height, const int32 Mode)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->SetScreenResolution(FIntPoint(Width, Height));
	Settings->SetFullscreenMode(static_cast<EWindowMode::Type>(Mode));
	return true;
}

// Change the current screen resolution
bool UUBPSaveLoadGraphicsSettings::ChangeScreenResolution(
	const int32 Width, const int32 Height, const int32 Mode)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->RequestResolutionChange(Width, Height, static_cast<EWindowMode::Type>(Mode), false);
	return true;
}

// Get the current video quality settings
bool UUBPSaveLoadGraphicsSettings::GetVideoQualitySettings(
	int32& AntiAliasing, int32& Effects, int32& PostProcess,
	int32& Resolution, int32& Shadow, int32& Texture, int32& ViewDistance)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	AntiAliasing = Settings->ScalabilityQuality.AntiAliasingQuality;
	Effects = Settings->ScalabilityQuality.EffectsQuality;
	PostProcess = Settings->ScalabilityQuality.PostProcessQuality;
	Resolution = Settings->ScalabilityQuality.ResolutionQuality;
	Shadow = Settings->ScalabilityQuality.ShadowQuality;
	Texture = Settings->ScalabilityQuality.TextureQuality;
	ViewDistance = Settings->ScalabilityQuality.ViewDistanceQuality;
	return true;
}

// Set the quality settings (not applied nor saved yet)
bool UUBPSaveLoadGraphicsSettings::SetVideoQualitySettings(
	const int32 AntiAliasing, const int32 Effects, const int32 PostProcess,
	const int32 Resolution, const int32 Shadow, const int32 Texture, const int32 ViewDistance)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->ScalabilityQuality.AntiAliasingQuality = AntiAliasing;
	Settings->ScalabilityQuality.EffectsQuality = Effects;
	Settings->ScalabilityQuality.PostProcessQuality = PostProcess;
	Settings->ScalabilityQuality.ResolutionQuality = Resolution;
	Settings->ScalabilityQuality.ShadowQuality = Shadow;
	Settings->ScalabilityQuality.TextureQuality = Texture;
	Settings->ScalabilityQuality.ViewDistanceQuality = ViewDistance;
	Settings->SaveSettings();

	return true;
}

// Check whether or not we have vertical sync enabled
bool UUBPSaveLoadGraphicsSettings::IsVSyncEnabled()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	return Settings->IsVSyncEnabled();
}


// Set the vertical sync flag
bool UUBPSaveLoadGraphicsSettings::SetVSyncEnabled(const bool VSync)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->SetVSyncEnabled(VSync);
	return true;
}

bool UUBPSaveLoadGraphicsSettings::SaveVideoModeAndQuality()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->ConfirmVideoMode();
	Settings->ApplyNonResolutionSettings();
	Settings->SaveSettings();
	return true;
}

// Revert to original video settings
bool UUBPSaveLoadGraphicsSettings::RevertVideoMode()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->RevertVideoMode();
	return true;
}