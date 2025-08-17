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
		const LSOnlineConfigType OnlineConfig = LSOnlineConfigType.OnlineSubsystemSteam;
		const bool bUseOSSV1 = (OnlineConfig == LSOnlineConfigType.OnlineSubsystemNull) || (OnlineConfig == LSOnlineConfigType.OnlineSubsystemEOS) || (OnlineConfig == LSOnlineConfigType.OnlineSubsystemSteam);
		
		PublicDefinitions.Add("LS_USE_OSSV1=" + (bUseOSSV1 ? "1" : "0"));
		PublicDefinitions.Add("LS_USE_ONLINESUBSYSTEMEOS=" + (OnlineConfig == LSOnlineConfigType.OnlineSubsystemEOS ? "1" : "0"));
		PublicDefinitions.Add("LS_USE_ONLINESUBSYSTEMSTEAM=" + (OnlineConfig == LSOnlineConfigType.OnlineSubsystemSteam ? "1" : "0"));
		PublicDefinitions.Add("LS_USE_ONLINESERVICESEPIC=" + (OnlineConfig == LSOnlineConfigType.OnlineServicesEpic ? "1" : "0"));
		
		PublicIncludePaths.AddRange(new [] { "LobbySample" });

		PublicDependencyModuleNames.AddRange([
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ApplicationCore",
			"EngineSettings",
			"NetCore",
			"CoreOnline",
			"UMG",
			"Slate",
			"SlateCore",
			"ImGui",
		]);
		
		if (bUseOSSV1)
		{
			PublicDependencyModuleNames.AddRange([
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"OnlineSubsystemNull",
				"OnlineSubsystemEOS" ,
			]);
		}
		else
		{
			PublicDependencyModuleNames.AddRange(["OnlineServicesInterface"]);
		}

		// https://dev.epicgames.com/documentation/en-us/unreal-engine/online-subsystem-steam-interface-in-unreal-engine#modulesetup
		// DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
		PublicDependencyModuleNames.AddRange(["OnlineSubsystemSteam"]);
	}
}
