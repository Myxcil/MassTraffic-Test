// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MassTraffic : ModuleRules
{
	public MassTraffic(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
			
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				// AI/MassAI Plugin Modules
				"MassEntity",
				// "StructUtils",
				"MassCommon",
				"MassMovement",
				"MassActors",
				"MassSpawner",
				"MassGameplayDebug",
				"MassSignals",
				"MassCrowd",
				"MassActors",
				"MassSpawner",
				"MassRepresentation",
				"MassReplication",
				"MassNavigation",
				"MassSimulation",
				"ZoneGraph",
                "MassGameplayDebug",
                "MassZoneGraphNavigation",
                "MassLOD",
                "MassAIBehavior",
				
				// Misc
				"AIModule",
				"Core",
				"Engine",
				"NetCore",
				"StateTreeModule",
				"ZoneGraph",
				"AnimToTexture",
				"ChaosVehicles",
				"ChaosVehiclesCore",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// Runtime/MassGameplay Plugin Modules
				"MassMovement",
				"MassReplication",
				"MassSimulation",
				
				// Misc
				"CoreUObject",
				"GameplayTasks",
				"RHI",
				"RenderCore",
				"Slate",
				"SlateCore",
				"PhysicsCore",
				"Chaos",
				"ChaosCore",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
		
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
				}
			);
		}
	}
}
