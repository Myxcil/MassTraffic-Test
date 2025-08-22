// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficInitTrafficVehiclesProcessor.h"
#include "MassTraffic.h"
#include "MassTrafficFragments.h"
#include "MassTrafficLaneChange.h"
#include "MassTrafficMovement.h"
#include "MassExecutionContext.h"
#include "MassReplicationSubsystem.h"
#include "MassReplicationFragments.h"
#include "MassRepresentationFragments.h"
#include "MassTrafficVehicleSimulationTrait.h"
#include "MassZoneGraphNavigationFragments.h"
#include "ZoneGraphTypes.h"


using namespace UE::MassTraffic;


UMassTrafficInitTrafficVehiclesProcessor::UMassTrafficInitTrafficVehiclesProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false;
}

void UMassTrafficInitTrafficVehiclesProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassTrafficRandomFractionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassTrafficVehicleControlFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassZoneGraphLaneLocationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassNetworkIDFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FMassTrafficVehicleVolumeParameters>();
	EntityQuery.AddSubsystemRequirement<UMassTrafficSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassReplicationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMassTrafficInitTrafficVehiclesProcessor::InitializeInternal(UObject& InOwner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(InOwner, EntityManager);

	MassRepresentationSubsystem = UWorld::GetSubsystem<UMassRepresentationSubsystem>(InOwner.GetWorld());
}

void UMassTrafficInitTrafficVehiclesProcessor::InitNetIds(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(MassProcessor_InitNetworkID_Run);

	check(EntityManager.GetWorld() && EntityManager.GetWorld()->GetNetMode() != NM_Client);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
		{
			UMassReplicationSubsystem& ReplicationSubsystem = Context.GetMutableSubsystemChecked<UMassReplicationSubsystem>();
			const TArrayView<FMassNetworkIDFragment> NetworkIDList = Context.GetMutableFragmentView<FMassNetworkIDFragment>();
			// the iterator is here for fragment writing breakpoint purposes
			FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator();
			for (FMassNetworkIDFragment& NetworkIDFragment : NetworkIDList)
			{
				NetworkIDFragment.NetID = ReplicationSubsystem.GetNextAvailableMassNetID();
				++EntityIt;
			}
		});
}

void UMassTrafficInitTrafficVehiclesProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	InitNetIds(EntityManager, Context);

	// Cast AuxData to required FMassTrafficVehiclesSpawnData
	const FInstancedStruct& AuxInput = Context.GetAuxData();
	if (!ensure(AuxInput.GetPtr<FMassTrafficVehiclesSpawnData>()))
	{
		return;
	}
	const FMassTrafficVehiclesSpawnData& VehiclesSpawnData = AuxInput.Get<FMassTrafficVehiclesSpawnData>();

	// Reset random stream used to seed FDataFragment_RandomFraction::RandomFraction
	RandomStream.Reset();

	// Init dynamic vehicle data 
	int32 VehicleIndex = 0;
	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& QueryContext)
	{
		UMassTrafficSubsystem& MassTrafficSubsystem = QueryContext.GetMutableSubsystemChecked<UMassTrafficSubsystem>();

		const FMassTrafficVehicleVolumeParameters& ObstacleParameters = QueryContext.GetConstSharedFragment<FMassTrafficVehicleVolumeParameters>();
		const TArrayView<FMassRepresentationFragment> RepresentationFragments = QueryContext.GetMutableFragmentView<FMassRepresentationFragment>();
		const TArrayView<FMassTrafficVehicleControlFragment> VehicleControlFragments = QueryContext.GetMutableFragmentView<FMassTrafficVehicleControlFragment>();
		const TArrayView<FMassZoneGraphLaneLocationFragment> LaneLocationFragments = QueryContext.GetMutableFragmentView<FMassZoneGraphLaneLocationFragment>();
		const TArrayView<FMassTrafficRandomFractionFragment> RandomFractionFragments = QueryContext.GetMutableFragmentView<FMassTrafficRandomFractionFragment>();
		const TArrayView<FTransformFragment> TransformFragments = QueryContext.GetMutableFragmentView<FTransformFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = QueryContext.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			check(VehiclesSpawnData.LaneLocations.IsValidIndex(VehicleIndex));
			
			FMassRepresentationFragment& RepresentationFragment = RepresentationFragments[EntityIt];
			FMassTrafficVehicleControlFragment& VehicleControlFragment = VehicleControlFragments[EntityIt];
			FMassZoneGraphLaneLocationFragment& LaneLocationFragment = LaneLocationFragments[EntityIt];
			FMassTrafficRandomFractionFragment& RandomFractionFragment = RandomFractionFragments[EntityIt];
			FTransformFragment& TransformFragment = TransformFragments[EntityIt];

			// Init random fraction
			RandomFractionFragment.RandomFraction = RandomStream.GetFraction();

			// Init noise input with random offset
			VehicleControlFragment.NoiseInput = RandomFractionFragment.RandomFraction * 10000.0f;

			// Init lane location fragment
			const FZoneGraphLaneLocation& LaneLocation = VehiclesSpawnData.LaneLocations[VehicleIndex];
			FZoneGraphTrafficLaneData& TrafficLaneData = MassTrafficSubsystem.GetMutableTrafficLaneDataChecked(LaneLocation.LaneHandle);
			LaneLocationFragment.LaneHandle = LaneLocation.LaneHandle;
			LaneLocationFragment.DistanceAlongLane = LaneLocation.DistanceAlongLane;
			LaneLocationFragment.LaneLength = TrafficLaneData.Length;
			
			// Cache inline lane data
			VehicleControlFragment.CurrentLaneConstData = TrafficLaneData.ConstData;

			// Make sure we aren't spawning a restricted vehicle on a non-trunk lane
			if (!TrunkVehicleLaneCheck(&TrafficLaneData, VehicleControlFragment))
			{
				UE_LOG(LogMassTraffic, Error, TEXT("InitTrafficVehicles - Vehicle %d is restricted to trunk lanes yet has been spawned on a non-trunk lane %s. Check vehicle type spawn lane filters to ensure this doesn't happen"),
					Context.GetEntity(EntityIt).Index, *LaneLocationFragment.LaneHandle.ToString());
			}

			// While we've already resolved CurrentTrafficLaneData here, we do a quick check
			// to see if it only has one next lane. In this case we can preemptively set that as our next lane
			if (TrafficLaneData.NextLanes.Num() == 1)
			{
				VehicleControlFragment.NextLane = TrafficLaneData.NextLanes[0];

				++VehicleControlFragment.NextLane->NumVehiclesApproachingLane;

				// While we're here, update downstream traffic density. 
				TrafficLaneData.UpdateDownstreamFlowDensity(MassTrafficSettings->DownstreamFlowDensityMixtureFraction);
			}
			
			// Consume available space on the assigned lane
			const float SpaceTakenByVehicleOnLane = GetSpaceTakenByVehicleOnLane(ObstacleParameters.HalfLength, RandomFractionFragment.RandomFraction, MassTrafficSettings->MinimumDistanceToNextVehicleRange);
			TrafficLaneData.AddVehicleOccupancy(SpaceTakenByVehicleOnLane);

			// Init TransformFragment
			TransformFragment.GetMutableTransform().SetRotation(FRotationMatrix::MakeFromX(LaneLocation.Direction).ToQuat());
			TransformFragment.GetMutableTransform().SetTranslation(LaneLocation.Position);

			// Seed RepresentationFragment.PrevTransform with initial transform
			RepresentationFragment.PrevTransform = TransformFragment.GetTransform();

			// Advance through spawn data
			++VehicleIndex;
		}
	});
}
