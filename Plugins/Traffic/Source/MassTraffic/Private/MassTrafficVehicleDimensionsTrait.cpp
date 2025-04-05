// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficVehicleDimensionsTrait.h"
#include "MassTrafficFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "MassEntityUtils.h"
#include "MassTrafficVehicleVolumeTrait.h"


void UMassTrafficVehicleDimensionsTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	// Add simulation config as shared fragment for length and width access.
	FMassTrafficVehicleVolumeParameters ObstacleConfig;
	ObstacleConfig.HalfLength = Params.HalfLength;
	ObstacleConfig.HalfWidth = Params.HalfWidth;

	const FConstSharedStruct ObstacleConfigSharedFragment = EntityManager.GetOrCreateConstSharedFragment(ObstacleConfig);
	BuildContext.AddConstSharedFragment(ObstacleConfigSharedFragment);
}
