// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficFindNextVehicleProcessor.h"
#include "MassTrafficFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassZoneGraphNavigationFragments.h"


UMassTrafficFindNextVehicleProcessor::UMassTrafficFindNextVehicleProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false;
}

void UMassTrafficFindNextVehicleProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassZoneGraphLaneLocationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassTrafficNextVehicleFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassTrafficSubsystem>(EMassFragmentAccess::ReadWrite);

	ProcessorRequirements.AddSubsystemRequirement<UMassTrafficSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMassTrafficFindNextVehicleProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UMassTrafficSubsystem& MassTrafficSubsystem = Context.GetMutableSubsystemChecked<UMassTrafficSubsystem>();

	// Gather all fragments
	TArray<FMassEntityHandle> AllVehicles;
	EntityQuery.ForEachEntityChunk(Context, [&](const FMassExecutionContext& QueryContext)
	{
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			AllVehicles.Add(QueryContext.GetEntity(EntityIt));
		}
	});
	if (AllVehicles.IsEmpty())
	{
		return;
	}

	// Sort first by lane, and then by distance
	AllVehicles.Sort(
		[&EntityManager](const FMassEntityHandle& EntityA, const FMassEntityHandle& EntityB)
		{
			const FMassZoneGraphLaneLocationFragment& A = EntityManager.GetFragmentDataChecked<FMassZoneGraphLaneLocationFragment>(EntityA);
			const FMassZoneGraphLaneLocationFragment& B = EntityManager.GetFragmentDataChecked<FMassZoneGraphLaneLocationFragment>(EntityB);

			if (A.LaneHandle == B.LaneHandle)
			{
				return A.DistanceAlongLane < B.DistanceAlongLane;
			}
			else if (A.LaneHandle.DataHandle == B.LaneHandle.DataHandle)
			{
				return A.LaneHandle.Index < B.LaneHandle.Index;
			}
			else
			{
				return A.LaneHandle.DataHandle.Index < B.LaneHandle.DataHandle.Index;
			}
		}
	);

	// Set Next pointers
	bool bTail = true;
	for (int32 EntityIt = 0; EntityIt < AllVehicles.Num() - 1; ++EntityIt)
	{
		const FMassEntityHandle& VehicleEntity = AllVehicles[EntityIt];
		FMassEntityView VehicleEntityView(EntityManager, VehicleEntity);
		FMassZoneGraphLaneLocationFragment& LaneLocationFragment = VehicleEntityView.GetFragmentData<FMassZoneGraphLaneLocationFragment>();
		FMassTrafficNextVehicleFragment& NextVehicleFragment = VehicleEntityView.GetFragmentData<FMassTrafficNextVehicleFragment>();
		
		const FMassEntityHandle& NextVehicleEntity = AllVehicles[EntityIt+1];
		const FMassZoneGraphLaneLocationFragment& NextLaneLocationFragment = EntityManager.GetFragmentDataChecked<FMassZoneGraphLaneLocationFragment>(NextVehicleEntity);

		// First in lane? 
		if (bTail)
		{
			if (FZoneGraphTrafficLaneData* TrafficLaneData = MassTrafficSubsystem.GetMutableTrafficLaneData(LaneLocationFragment.LaneHandle))
			{
				TrafficLaneData->TailVehicle = VehicleEntity;
			}
			bTail = false;
		}

		if (LaneLocationFragment.LaneHandle == NextLaneLocationFragment.LaneHandle)
		{
			NextVehicleFragment.SetNextVehicle(VehicleEntity, NextVehicleEntity);
		}
		else
		{
			NextVehicleFragment.UnsetNextVehicle();

			bTail = true;
		}
	}

	// Process last in list
	const FMassEntityHandle& LastVehicleEntity = AllVehicles.Last();
	const FMassEntityView LastVehicleEntityView(EntityManager, LastVehicleEntity);
	const FMassZoneGraphLaneLocationFragment& LastLaneLocationFragment = LastVehicleEntityView.GetFragmentData<FMassZoneGraphLaneLocationFragment>();
	FMassTrafficNextVehicleFragment& LastNextVehicleFragment = LastVehicleEntityView.GetFragmentData<FMassTrafficNextVehicleFragment>();

	// Last in list, implicitly has no next vehicle
	LastNextVehicleFragment.UnsetNextVehicle();

	// It may be first in its lane though?
	if (bTail)
	{
		if (FZoneGraphTrafficLaneData* TrafficLaneData = MassTrafficSubsystem.GetMutableTrafficLaneData(LastLaneLocationFragment.LaneHandle))
		{
			TrafficLaneData->TailVehicle = LastVehicleEntity;
		}
	}
	
	// Now that all the vehicles have been assigned to their lanes, go through and connect the last vehicle on each
	// lane to the closest first vehicle in the next connected lanes 
	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& QueryContext)
	{
		const UMassTrafficSubsystem& LocalMassTrafficSubsystem = QueryContext.GetSubsystemChecked<UMassTrafficSubsystem>();
		const TConstArrayView<FMassZoneGraphLaneLocationFragment> LaneLocationFragments = QueryContext.GetFragmentView<FMassZoneGraphLaneLocationFragment>();
		const TArrayView<FMassTrafficNextVehicleFragment> NextVehicleFragments = QueryContext.GetMutableFragmentView<FMassTrafficNextVehicleFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = QueryContext.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FMassZoneGraphLaneLocationFragment& LaneLocationFragment = LaneLocationFragments[EntityIt];
			FMassTrafficNextVehicleFragment& NextVehicleFragment = NextVehicleFragments[EntityIt];
        	
			// Is this the last vehicle in it's lane?
			if (!NextVehicleFragment.HasNextVehicle())
			{
				if (const FZoneGraphTrafficLaneData* TrafficLaneData = LocalMassTrafficSubsystem.GetTrafficLaneData(LaneLocationFragment.LaneHandle))
				{
					// Find the closest tail vehicle across all connected lanes
					FMassEntityHandle ClosestTail = FMassEntityHandle();
					float ClosestTailDistance = TNumericLimits<float>::Max();
	            	
					for (const FZoneGraphTrafficLaneData* NextTrafficLaneData : TrafficLaneData->NextLanes)
					{
						if (NextTrafficLaneData->TailVehicle.IsSet())
						{
							const FMassZoneGraphLaneLocationFragment& TailVehicleLaneLocation = EntityManager.GetFragmentDataChecked<FMassZoneGraphLaneLocationFragment>(NextTrafficLaneData->TailVehicle);
							if (TailVehicleLaneLocation.DistanceAlongLane < ClosestTailDistance)
							{
								ClosestTailDistance = TailVehicleLaneLocation.DistanceAlongLane; 
								ClosestTail = NextTrafficLaneData->TailVehicle;
							}
						}
					}
 
					if (ClosestTail.IsSet())
					{
						// Set the closest subsequent tail as this vehicles Next
						NextVehicleFragment.SetNextVehicle(QueryContext.GetEntity(EntityIt), ClosestTail);
					}
				}
			}
		}
	});
}
