// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficLights.h"

#include "EngineUtils.h"
#include "MassTraffic.h"
#include "MassTrafficDebugHelpers.h"
#include "MassTrafficLightActor.h"

#if WITH_EDITOR
#include "Engine/StaticMesh.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/ScopedSlowTask.h"
#endif

#if WITH_EDITOR

FVector TransformPositionFromHoudini(const FVector& HoudiniPosition)
{
	static const FTransform HoudiniToUEConversionTransform(FRotator(0.0f, 0.0f, -90.0f), FVector::ZeroVector, FVector(1.0f, 1.0f, -1.0f));
	return HoudiniToUEConversionTransform.TransformPosition(HoudiniPosition);
}

FQuat TransformRotationFromHoudini(const FQuat& HoudiniRotation)
{
	return FQuat(HoudiniRotation.X, HoudiniRotation.Z, -HoudiniRotation.Y, HoudiniRotation.W);
}

void UMassTrafficLightInstancesDataAsset::PopulateTrafficLightsFromMap()
{
	TrafficLights.Empty();
	NumTrafficLights = 0;

	const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	ensure(World);

	// Iterate over all proxy actors and create the data similar to the point cloud 
	for(TActorIterator<AMassTrafficLightActor> It(World); It; ++It)
	{
		const FTransform& Transform = It->GetTransform();

		UE_LOG(LogTemp, Warning, TEXT("Found something..."))
		
		int16 TrafficLightTypeIndex = It->GetTrafficLightTypeIndex();
		if (TrafficLightTypeIndex == INDEX_NONE)
		{
			TrafficLightTypeIndex = FMath::RandHelper(TrafficLightTypesData->TrafficLightTypes.Num());
		}

		const FVector ControlledIntersectionSideMidpoint = Transform.GetLocation();

		const FVector TrafficLightPosition = Transform.GetLocation();
		const FQuat TrafficLightRotation = Transform.GetRotation();
	
		float TrafficLightZRotation;
		FVector TrafficLightXDirection_Debug = FVector::ZeroVector;
		{
			const FRotator Rotator(TrafficLightRotation);
			const FVector Euler = Rotator.Euler();
			TrafficLightZRotation = Euler.Z;
			TrafficLightXDirection_Debug = Rotator.RotateVector(FVector::XAxisVector);
		}

		const FMassTrafficLightInstanceDesc TrafficLightDetail(TrafficLightPosition, TrafficLightZRotation, ControlledIntersectionSideMidpoint, TrafficLightTypeIndex);
		TrafficLights.Add(TrafficLightDetail);

		// DEBUG
#if ENABLE_DRAW_DEBUG
		if (GDebugMassTraffic)
		{
			UE::MassTraffic::DrawDebugTrafficLight(GWorld, TrafficLightPosition, TrafficLightXDirection_Debug, &ControlledIntersectionSideMidpoint, FColor::Yellow, FColor::Yellow, FColor::Yellow, FColor::Yellow, false, 20.0f);
		}
#endif
	}

	NumTrafficLights = TrafficLights.Num();
	
	Modify();
}

#endif
