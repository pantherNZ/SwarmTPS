// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Swarm : ModuleRules
{
    public Swarm(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[] { 
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "RHI",
                "UMG", 
                "Slate", 
                "SlateCore",
                "NavigationSystem"
            }
        );
        
        PublicDependencyModuleNames.AddRange(new string[] { 
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });
        
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
    }
}
