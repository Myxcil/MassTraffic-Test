// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficParkedVehicles.h"

#include "EngineUtils.h"
#include "MassTraffic.h"
#include "MassTrafficDebugHelpers.h"
#include "MassTrafficParkingSpotActor.h"
#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Misc/ScopedSlowTask.h"
#endif

#if WITH_EDITOR

void UMassTrafficParkingSpacesDataAsset::PopulateParkingSpacesFromMap()
{
	TypedParkingSpaces.Empty();
	NumParkingSpaces = 0;

	const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	ensure(World);

	TMap<FName, TArray<const AMassTrafficParkingSpotActor*>> ParkingSpotsMap;
	for(TActorIterator<AMassTrafficParkingSpotActor> It(World); It; ++It)
	{
		FName ParkingSpaceType = It->GetParkingSpaceType();
		if (!ParkingSpaceType.IsValid())
		{
			ParkingSpaceType = DefaultParkingSpaceType;
		}
		ParkingSpotsMap.FindOrAdd(ParkingSpaceType).Add(*It);
	}

	for(const auto& ParkingSpaceActorIt : ParkingSpotsMap)
	{
		FMassTrafficTypedParkingSpaces& TrafficTypedParkingSpace = TypedParkingSpaces.AddDefaulted_GetRef();
		TrafficTypedParkingSpace.Name = ParkingSpaceActorIt.Key;
		for(const AMassTrafficParkingSpotActor* ParkingSpotActor : ParkingSpaceActorIt.Value)
		{
			TrafficTypedParkingSpace.ParkingSpaces.Add(ParkingSpotActor->GetTransform());
		}
		TrafficTypedParkingSpace.NumParkingSpaces = TrafficTypedParkingSpace.ParkingSpaces.Num();
	}
	
	// Randomly shuffle the parking space transforms so we can select the first NumParkedVehicles and get a random
	// distribution of parking spaces 
	if (bShuffleParkingSpaces)
	{
		for (FMassTrafficTypedParkingSpaces& TypedParkingSpacesDesc : TypedParkingSpaces)
		{
			Algo::RandomShuffle(TypedParkingSpacesDesc.ParkingSpaces);
		}
	}

	// Update counts
	for (FMassTrafficTypedParkingSpaces& TypedParkingSpacesDesc : TypedParkingSpaces)
	{
		TypedParkingSpacesDesc.NumParkingSpaces = TypedParkingSpacesDesc.ParkingSpaces.Num();
		NumParkingSpaces += TypedParkingSpacesDesc.NumParkingSpaces;
	}
	
	MarkPackageDirty();
}

#endif // WITH_EDITOR
