// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LobbySample : ModuleRules
{
	public enum LSOnlineConfigType
	{
		None,
		OnlineServicesEpic,
		OnlineSubsystemNull,
		OnlineSubsystemSteam,
		OnlineSubsystemEOS,
	}

	public LobbySample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// if you change this you will also need to enable/disable the OnlineSubsystem and OnlineServices plugins and the BuildPackages.ps1 script
		const LSOnlineConfigType OnlineConfig = LSOnlineConfigType.OnlineSubsystemEOS;
		const bool bUseOSSV1 = (OnlineConfig == LSOnlineConfigType.OnlineSubsystemNull) || (OnlineConfig == LSOnlineConfigType.OnlineSubsystemEOS) || (OnlineConfig == LSOnlineConfigType.OnlineSubsystemSteam);
		
		PublicDefinitions.Add("LS_USE_OSSV1=" + (bUseOSSV1 ? "1" : "0"));
		PublicDefinitions.Add("LS_USE_ONLINESUBSYSTEMEOS=" + (OnlineConfig == LSOnlineConfigType.OnlineSubsystemEOS ? "1" : "0"));
		PublicDefinitions.Add("LS_USE_ONLINESUBSYSTEMSTEAM=" + (OnlineConfig == LSOnlineConfigType.OnlineSubsystemSteam ? "1" : "0"));
		PublicDefinitions.Add("LS_USE_ONLINESERVICESEPIC=" + (OnlineConfig == LSOnlineConfigType.OnlineServicesEpic ? "1" : "0"));
		
		PublicIncludePaths.AddRange(new [] { "LobbySample" });

		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ApplicationCore", "EngineSettings" });
		PublicDependencyModuleNames.AddRange(new [] { "NetCore", "CoreOnline" });
		
		if (bUseOSSV1)
		{
			PublicDependencyModuleNames.AddRange(new [] { "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemNull", "OnlineSubsystemEOS" , "OnlineSubsystemSteam" });
		}
		else
		{
			PublicDependencyModuleNames.AddRange(new [] { "OnlineServicesInterface" });
		}

		// https://dev.epicgames.com/documentation/en-us/unreal-engine/online-subsystem-steam-interface-in-unreal-engine#modulesetup
		// DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
		PublicDependencyModuleNames.AddRange(new [] { "OnlineSubsystemSteam" });
		
		PublicDependencyModuleNames.AddRange(new [] { "UMG" });
		PublicDependencyModuleNames.AddRange(new [] { "Slate", "SlateCore" });

		PublicDependencyModuleNames.AddRange(new [] { "ImGui" });
		// Tell the compiler we want to import the ImPlot symbols when linking against ImGui plugin 
		PrivateDefinitions.Add(string.Format("IMPLOT_API=DLLIMPORT"));
	}
}
