// Copyright Epic Games, Inc. All Rights Reserved.


#include "MassTrafficEditor.h"

#include "MassTrafficIntersectionComponent.h"
#include "MassTrafficIntersectionComponentVisualizer.h"
#include "MassTrafficParkingSpotComponent.h"
#include "MassTrafficParkingSpotComponentVisualizer.h"
#include "MassTrafficPathFollower.h"
#include "MassTrafficPathFollowerVisualizer.h"
#include "MassTrafficTrackNearVehicles.h"
#include "MassTrafficTrackNearVehiclesVisualizer.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "HAL/IConsoleManager.h"

#define LOCTEXT_NAMESPACE "FMassTrafficEditorModule"


// CVars
int32 GDebugMassTrafficEditor = 0;
FAutoConsoleVariableRef CVarMassTrafficEditorDebug(
    TEXT("MassTrafficEditor.Debug"),
    GDebugMassTrafficEditor,
    TEXT("MassTraffic debug mode.\n")
    TEXT("0 = Off (default)\n")
    TEXT("1 = Show debug messages"),
    ECVF_Cheat
    );


void FMassTrafficEditorModule::StartupModule()
{
	RegisterComponentVisualizer(UMassTrafficPathFollower::StaticClass()->GetFName(), MakeShareable(new FMassTrafficPathFollowerVisualizer));
	RegisterComponentVisualizer(UMassTrafficTrackNearVehicles::StaticClass()->GetFName(), MakeShareable(new FMassTrafficTrackNearVehiclesVisualizer));
	RegisterComponentVisualizer(UMassTrafficParkingSpotComponent::StaticClass()->GetFName(), MakeShareable(new FMassTrafficParkingSpotComponentVisualizer));
	RegisterComponentVisualizer(UMassTrafficIntersectionComponent::StaticClass()->GetFName(), MakeShareable(new FMassTrafficIntersectionComponentVisualizer));
}

void FMassTrafficEditorModule::ShutdownModule()
{
	
	if (GEngine)
	{
		// Iterate over all class names we registered for
		for (const FName ClassName : RegisteredComponentClassNames)
		{
			GUnrealEd->UnregisterComponentVisualizer(ClassName);
		}
	}
}

void FMassTrafficEditorModule::RegisterComponentVisualizer(const FName& ComponentClassName, const TSharedPtr<FComponentVisualizer>& Visualizer)
{
	if (GUnrealEd)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}
	RegisteredComponentClassNames.Add(ComponentClassName);
	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMassTrafficEditorModule, MassTrafficEditor)

DEFINE_LOG_CATEGORY(LogMassTrafficEditor);
