// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GunLock : ModuleRules
{
    public GunLock(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(
           new string[]
                {
				    "OnlineSubsystem",
				    "OnlineSubsystemUtils",
			    }
                );

        PrivateDependencyModuleNames.AddRange(
                new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputCore",
					"HeadMountedDisplay",
				}
                );

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
	}
}
