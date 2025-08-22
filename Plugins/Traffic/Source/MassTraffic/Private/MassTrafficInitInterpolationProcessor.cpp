// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficInitInterpolationProcessor.h"
#include "MassTraffic.h"
#include "MassTrafficChooseNextLaneProcessor.h"
#include "MassTrafficFragments.h"
#include "MassTrafficInterpolation.h"
#include "MassExecutionContext.h"
#include "DrawDebugHelpers.h"
#include "MassZoneGraphNavigationFragments.h"
#include "ZoneGraphSubsystem.h"
#include "MassCommonFragments.h"


UMassTrafficInitInterpolationProcessor::UMassTrafficInitInterpolationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false;
}

void UMassTrafficInitInterpolationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassZoneGraphLaneLocationFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.AddRequirement<FMassTrafficInterpolationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UZoneGraphSubsystem>(EMassFragmentAccess::ReadOnly);
}

void UMassTrafficInitInterpolationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [&, World = EntityManager.GetWorld()](FMassExecutionContext& QueryContext)
	{
		const UZoneGraphSubsystem& ZoneGraphSubsystem = QueryContext.GetSubsystemChecked<UZoneGraphSubsystem>();

		const TConstArrayView<FMassZoneGraphLaneLocationFragment> LaneLocationFragments = QueryContext.GetFragmentView<FMassZoneGraphLaneLocationFragment>();
		const TArrayView<FMassTrafficInterpolationFragment> VehicleMovementInterpolationFragments = QueryContext.GetMutableFragmentView<FMassTrafficInterpolationFragment>();
		const TArrayView<FTransformFragment> TransformFragments = QueryContext.GetMutableFragmentView<FTransformFragment>();
		
		for (FMassExecutionContext::FEntityIterator EntityIt = QueryContext.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FMassZoneGraphLaneLocationFragment& LaneLocationFragment = LaneLocationFragments[EntityIt];
			FMassTrafficInterpolationFragment& VehicleMovementInterpolationFragment = VehicleMovementInterpolationFragments[EntityIt];
			FTransformFragment& TransformFragment = TransformFragments[EntityIt];

			const FZoneGraphStorage* ZoneGraphStorage = ZoneGraphSubsystem.GetZoneGraphStorage(LaneLocationFragment.LaneHandle.DataHandle);
			check(ZoneGraphStorage);
			
			// Interpolate initial transform
			UE::MassTraffic::InterpolatePositionAndOrientationAlongLane(*ZoneGraphStorage, LaneLocationFragment.LaneHandle.Index, LaneLocationFragment.DistanceAlongLane, ETrafficVehicleMovementInterpolationMethod::Linear, VehicleMovementInterpolationFragment.LaneLocationLaneSegment, TransformFragment.GetMutableTransform());

			// Debug
			if (GMassTrafficDebugInterpolation)
			{
				DrawDebugPoint(World, VehicleMovementInterpolationFragment.LaneLocationLaneSegment.StartPoint, 20.0f, FColor::Red);
				DrawDebugPoint(World, VehicleMovementInterpolationFragment.LaneLocationLaneSegment.StartControlPoint, 20.0f, FColor::Green);
				DrawDebugPoint(World, VehicleMovementInterpolationFragment.LaneLocationLaneSegment.EndControlPoint, 20.0f, FColor::Blue);
				DrawDebugPoint(World, VehicleMovementInterpolationFragment.LaneLocationLaneSegment.EndPoint, 20.0f, FColor::Cyan);
			}
		}
	});
}
