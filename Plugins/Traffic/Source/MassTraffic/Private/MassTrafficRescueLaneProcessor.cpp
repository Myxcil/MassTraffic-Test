// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficRescueLaneProcessor.h"

#include "MassExecutionContext.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MassTrafficVehicleVolumeTrait.h"
#include "MassExternalSubsystemTraits.h" // KEEP THIS UNDER ALL CIRCUMSTANCES OR ELSE COMPILE WILL FAIL!!!!


UMassTrafficRescueLaneProcessor::UMassTrafficRescueLaneProcessor() :
	EMVehicleQuery(*this),
	VehicleQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionOrder.ExecuteInGroup = UE::MassTraffic::ProcessorGroupNames::PreVehicleBehavior;
	ExecutionOrder.ExecuteAfter.Add(UE::MassTraffic::ProcessorGroupNames::FrameStart);
	ExecutionOrder.ExecuteAfter.Add(UE::MassTraffic::ProcessorGroupNames::VehicleSimulationLOD);
}

void UMassTrafficRescueLaneProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EMVehicleQuery.AddTagRequirement<FMassTrafficObstacleTag>(EMassFragmentPresence::All);
	EMVehicleQuery.AddTagRequirement<FMassTrafficEmergencyTag>(EMassFragmentPresence::All);
	EMVehicleQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EMVehicleQuery.AddConstSharedRequirement<FMassTrafficVehicleVolumeParameters>();
	
	VehicleQuery.AddTagRequirement<FMassTrafficVehicleTag>(EMassFragmentPresence::Any);
	VehicleQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	VehicleQuery.AddRequirement<FMassZoneGraphLaneLocationFragment>(EMassFragmentAccess::ReadOnly);
	VehicleQuery.AddRequirement<FMassTrafficVehicleControlFragment>(EMassFragmentAccess::ReadWrite);
	VehicleQuery.AddRequirement<FMassTrafficVehicleLightsFragment>(EMassFragmentAccess::ReadWrite);

	ProcessorRequirements.AddSubsystemRequirement<UMassTrafficSubsystem>(EMassFragmentAccess::ReadOnly);
}

void UMassTrafficRescueLaneProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const UMassTrafficSubsystem& MassTrafficSubsystem = Context.GetSubsystemChecked<UMassTrafficSubsystem>();
	
	// first, get all the active EM vehicles
	TArray<FMassEntityHandle> EMVehicles;
	EMVehicleQuery.ForEachEntityChunk( Context, [&](const FMassExecutionContext& QueryContext)
	{
		const int32 NumEntities = QueryContext.GetNumEntities();
		for(int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			EMVehicles.Add(QueryContext.GetEntity(EntityIndex));
		}
	});

	const int32 NumEMVehicles = EMVehicles.Num();
	const float EMRecognitionDistanceSquared = MassTrafficSettings->RescueLaneEMRecognitionDistance * MassTrafficSettings->RescueLaneEMRecognitionDistance;

	VehicleQuery.ForEachEntityChunk( Context, [&](FMassExecutionContext& QueryContext)
	{
		const TConstArrayView<FTransformFragment> TransformFragments = QueryContext.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassZoneGraphLaneLocationFragment> LaneLocationFragments = QueryContext.GetFragmentView<FMassZoneGraphLaneLocationFragment>();
		const TArrayView<FMassTrafficVehicleControlFragment> VehicleControlFragments = QueryContext.GetMutableFragmentView<FMassTrafficVehicleControlFragment>();
		const TArrayView<FMassTrafficVehicleLightsFragment> VehicleLightsFragments  = QueryContext.GetMutableFragmentView<FMassTrafficVehicleLightsFragment>();
	
		const int32 NumEntities = QueryContext.GetNumEntities();
		for(int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			// only check if the vehicle is NOT an emergency vehicle
			const FTransformFragment& TransformFragment = TransformFragments[EntityIndex];
			const FTransform& EntityTransform = TransformFragment.GetTransform();
			const FVector EntityFwd = EntityTransform.GetRotation().GetForwardVector();

			// find nearest emergency vehicle that can make this entity to get out of the way
			double SqMinDist = EMRecognitionDistanceSquared;

			FMassEntityHandle NearestEMVehicle;
			FVector NearestEMPosition = FVector(0,0,0);
			for(int32 EMIndex = 0; EMIndex < NumEMVehicles; ++EMIndex)
			{
				const FMassEntityHandle EMHandle = EMVehicles[EMIndex];
			
				const FTransformFragment& EMTransformFragment = EntityManager.GetFragmentDataChecked<FTransformFragment>(EMHandle);
				const FTransform& EMTransform = EMTransformFragment.GetTransform();
				const FVector EMForward = EMTransform.GetRotation().GetForwardVector();

				if (FVector::DotProduct(EntityFwd, EMForward) > 0)
				{
					const FVector EMPosition = EMTransform.GetLocation();
					const FVector EntityPosition = EntityTransform.GetLocation();

					const double SqDist = FVector::DistSquared(EMPosition, EntityPosition); 
					if (SqDist < SqMinDist)
					{
						SqMinDist = SqDist;
						NearestEMVehicle = EMHandle;
						NearestEMPosition = EMPosition;
					}
				}
			}
			
			FMassTrafficVehicleControlFragment& VehicleControlFragment = VehicleControlFragments[EntityIndex];

			if (NearestEMVehicle.IsValid())
			{
				const FMassZoneGraphLaneLocationFragment& LaneLocationFragment= LaneLocationFragments[EntityIndex];
				const FZoneGraphTrafficLaneData* TrafficLaneData = MassTrafficSubsystem.GetTrafficLaneData(LaneLocationFragment.LaneHandle);
				if (TrafficLaneData && TrafficLaneData->bIsEmergencyLane)
				{
					VehicleControlFragment.EmergencyOffset = TrafficLaneData->bIsRightMostLane ? 1.0f : -1.0f;
				}
				else
				{
					const FVector ToLane = NearestEMPosition - EntityTransform.GetLocation();
					const FVector Cross = FVector::CrossProduct(EntityFwd, ToLane);
					VehicleControlFragment.EmergencyOffset = -FMath::Sign(FVector::DotProduct(Cross, FVector::UpVector));
				}
			}
			else
			{
				VehicleControlFragment.EmergencyOffset = 0.0f;
			}

			FMassTrafficVehicleLightsFragment& VehicleLightsFragment = VehicleLightsFragments[EntityIndex];
			const bool bWarnLightsOn = VehicleControlFragment.EmergencyOffset != 0;
			VehicleLightsFragment.bLeftTurnSignalLights = bWarnLightsOn;
			VehicleLightsFragment.bRightTurnSignalLights = bWarnLightsOn;
		}
	});
}
