// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficInitTrafficVehicleSpeedProcessor.h"
#include "MassTrafficFragments.h"
#include "MassTrafficMovement.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassTrafficUtils.h"
#include "MassZoneGraphNavigationFragments.h"


UMassTrafficInitTrafficVehicleSpeedProcessor::UMassTrafficInitTrafficVehicleSpeedProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false;
}

void UMassTrafficInitTrafficVehicleSpeedProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassTrafficRandomFractionFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassTrafficObstacleAvoidanceFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassZoneGraphLaneLocationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassTrafficVehicleControlFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassTrafficInitTrafficVehicleSpeedProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Advance agents
	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& QueryContext)
	{
		const TConstArrayView<FMassTrafficRandomFractionFragment> RandomFractionFragments = QueryContext.GetFragmentView<FMassTrafficRandomFractionFragment>();
		const TConstArrayView<FMassTrafficObstacleAvoidanceFragment> AvoidanceFragments = QueryContext.GetFragmentView<FMassTrafficObstacleAvoidanceFragment>();
		const TConstArrayView<FAgentRadiusFragment> RadiusFragments = QueryContext.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FMassZoneGraphLaneLocationFragment> LaneLocationFragments = QueryContext.GetFragmentView<FMassZoneGraphLaneLocationFragment>();
		const TArrayView<FMassTrafficVehicleControlFragment> VehicleControlFragments = QueryContext.GetMutableFragmentView<FMassTrafficVehicleControlFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = QueryContext.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FMassTrafficRandomFractionFragment& RandomFractionFragment = RandomFractionFragments[EntityIt];
			const FAgentRadiusFragment& AgentRadiusFragment = RadiusFragments[EntityIt];
			const FMassTrafficObstacleAvoidanceFragment& AvoidanceFragment = AvoidanceFragments[EntityIt];
			const FMassZoneGraphLaneLocationFragment& LaneLocationFragment = LaneLocationFragments[EntityIt];
			FMassTrafficVehicleControlFragment& VehicleControlFragment = VehicleControlFragments[EntityIt];

			
			// Compute stable distance based noise
			const float NoiseValue = UE::MassTraffic::CalculateNoiseValue(VehicleControlFragment.NoiseInput, MassTrafficSettings->NoisePeriod);

			// Calculate varied speed limit
			const float SpeedLimit = UE::MassTraffic::GetSpeedLimitAlongLane(LaneLocationFragment.LaneLength,
				VehicleControlFragment.CurrentLaneConstData.SpeedLimit,
				VehicleControlFragment.CurrentLaneConstData.AverageNextLanesSpeedLimit,
				LaneLocationFragment.DistanceAlongLane, VehicleControlFragment.Speed, MassTrafficSettings->SpeedLimitBlendTime
			);
			const float VariedSpeedLimit = UE::MassTraffic::VarySpeedLimit(SpeedLimit, MassTrafficSettings->SpeedLimitVariancePct, MassTrafficSettings->SpeedVariancePct, RandomFractionFragment.RandomFraction, NoiseValue);
			
			// Should stop?
			bool bRequestDifferentNextLane = false;
			bool bVehicleCantStopAtLaneExit = false;
			bool bIsFrontOfVehicleBeyondEndOfLane = false;
			bool bNoNext = false;
			bool bNoRoom = false;
			const bool bMustStopAtLaneExit = UE::MassTraffic::ShouldStopAtLaneExit(
				LaneLocationFragment.DistanceAlongLane,
				VehicleControlFragment.Speed,
				AgentRadiusFragment.Radius,
				RandomFractionFragment.RandomFraction,
				LaneLocationFragment.LaneLength,
				VehicleControlFragment.NextLane,
				MassTrafficSettings->MinimumDistanceToNextVehicleRange,
				EntityManager,
				/*out*/bRequestDifferentNextLane,
				/*in/out*/bVehicleCantStopAtLaneExit,
				/*out*/bIsFrontOfVehicleBeyondEndOfLane,
				/*out*/bNoRoom,
				/*out*/bNoNext,
				MassTrafficSettings->StandardTrafficPrepareToStopSeconds
			);
			
			// CalculateTargetSpeed has time based variables that use the current speed to convert times
			// to distances. So these eval to non-zero we use VariedSpeedLimit as our stand in Current Speed when
			// computing initial speed.
			const float BaseSpeed = VariedSpeedLimit;

			// CalculateTargetSpeed relies on already having a current speed value set that it can
			//		 use to compute the distance to start braking for avoidance.
			const float TargetSpeed = UE::MassTraffic::CalculateTargetSpeed(
				LaneLocationFragment.DistanceAlongLane,
				BaseSpeed, 
				AvoidanceFragment.DistanceToNext,
				AvoidanceFragment.TimeToCollidingObstacle,
				AvoidanceFragment.DistanceToCollidingObstacle,
				AgentRadiusFragment.Radius,
				RandomFractionFragment.RandomFraction,
				LaneLocationFragment.LaneLength,
				VariedSpeedLimit,
				MassTrafficSettings->IdealTimeToNextVehicleRange,
				MassTrafficSettings->MinimumDistanceToNextVehicleRange,
				MassTrafficSettings->NextVehicleAvoidanceBrakingPower,
				MassTrafficSettings->ObstacleAvoidanceBrakingTimeRange,
				MassTrafficSettings->MinimumDistanceToObstacleRange,
				MassTrafficSettings->ObstacleAvoidanceBrakingPower,
				MassTrafficSettings->StopSignBrakingTime,
				MassTrafficSettings->StoppingDistanceRange,
				MassTrafficSettings->StopSignBrakingPower,
				bMustStopAtLaneExit
			);

			// Init speed to pure target speed
			VehicleControlFragment.Speed = TargetSpeed;
		}
	});
}