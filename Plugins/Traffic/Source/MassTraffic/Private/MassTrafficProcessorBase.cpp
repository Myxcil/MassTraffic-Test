// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficProcessorBase.h"
#include "MassCommonUtils.h"
#include "ZoneGraphSubsystem.h" 

void UMassTrafficProcessorBase::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	// Get settings
	MassTrafficSettings = GetDefault<UMassTrafficSettings>();

	LogOwner = UWorld::GetSubsystem<UMassTrafficSubsystem>(Owner.GetWorld());

	// Seed random stream
	const int32 TrafficRandomSeed = UE::Mass::Utils::OverrideRandomSeedForTesting(MassTrafficSettings->RandomSeed);
	if (TrafficRandomSeed >= 0 || UE::Mass::Utils::IsDeterministic())
	{
		RandomStream.Initialize(TrafficRandomSeed);
	}
	else
	{
		RandomStream.GenerateNewSeed();
	}
}
