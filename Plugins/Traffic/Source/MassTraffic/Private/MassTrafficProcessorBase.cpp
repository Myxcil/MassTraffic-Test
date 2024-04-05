// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficProcessorBase.h"

#include "ZoneGraphSubsystem.h" 

void UMassTrafficProcessorBase::Initialize(UObject& InOwner)
{
	Super::Initialize(InOwner);

	// Get settings
	MassTrafficSettings = GetDefault<UMassTrafficSettings>();

	LogOwner = UWorld::GetSubsystem<UMassTrafficSubsystem>(InOwner.GetWorld());

	// Seed random stream
	const int32 TrafficRandomSeed = MassTrafficSettings->RandomSeed;
	if (TrafficRandomSeed >= 0)
	{
		RandomStream.Initialize(TrafficRandomSeed);
	}
	else
	{
		RandomStream.GenerateNewSeed();
	}
}
