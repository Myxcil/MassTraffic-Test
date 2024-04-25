// Fill out your copyright notice in the Description page of Project Settings.


#include "MassTrafficRescueLaneProcessor.h"

#include "MassExecutionContext.h"
#include "MassZoneGraphNavigationFragments.h"



UMassTrafficRescueLaneProcessor::UMassTrafficRescueLaneProcessor() :
	EMVehicleQuery(*this),
	VehicleQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionOrder.ExecuteInGroup = UE::MassTraffic::ProcessorGroupNames::PreVehicleBehavior;
	ExecutionOrder.ExecuteAfter.Add(UE::MassTraffic::ProcessorGroupNames::FrameStart);
	ExecutionOrder.ExecuteAfter.Add(UE::MassTraffic::ProcessorGroupNames::VehicleSimulationLOD);
}

void UMassTrafficRescueLaneProcessor::ConfigureQueries()
{
	EMVehicleQuery.AddTagRequirement<FMassTrafficEMVehicleTag>(EMassFragmentPresence::Any);
	EMVehicleQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	VehicleQuery.AddTagRequirement<FMassTrafficVehicleTag>(EMassFragmentPresence::Any);
	VehicleQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	VehicleQuery.AddRequirement<FMassZoneGraphLaneLocationFragment>(EMassFragmentAccess::ReadOnly);
	VehicleQuery.AddRequirement<FMassTrafficVehicleControlFragment>(EMassFragmentAccess::ReadWrite);
	VehicleQuery.AddRequirement<FMassTrafficVehicleLightsFragment>(EMassFragmentAccess::ReadWrite);

	ProcessorRequirements.AddSubsystemRequirement<UMassTrafficSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMassTrafficRescueLaneProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const UMassTrafficSubsystem& MassTrafficSubsystem = Context.GetMutableSubsystemChecked<UMassTrafficSubsystem>();
	
	// first, get all the active EM vehicles
	TArray<FMassEntityHandle> EMVehicles;
	EMVehicleQuery.ForEachEntityChunk(EntityManager, Context, [&](const FMassExecutionContext& QueryContext)
	{
		const int32 NumEntities = QueryContext.GetNumEntities();
		for(int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			EMVehicles.Add(QueryContext.GetEntity(EntityIndex));
		}
	});

	const int32 NumEMVehicles = EMVehicles.Num();
	constexpr double EMRecognitionDistance = 5000.0f;
	constexpr double EMRecognitionDistanceSquared = EMRecognitionDistance * EMRecognitionDistance;
	
	VehicleQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& QueryContext)
	{
		const TConstArrayView<FTransformFragment> TransformFragments = QueryContext.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassZoneGraphLaneLocationFragment> LaneLocationFragments = QueryContext.GetFragmentView<FMassZoneGraphLaneLocationFragment>();
		const TArrayView<FMassTrafficVehicleControlFragment> VehicleControlFragments = QueryContext.GetMutableFragmentView<FMassTrafficVehicleControlFragment>();
		const TArrayView<FMassTrafficVehicleLightsFragment> VehicleLightsFragments  = QueryContext.GetMutableFragmentView<FMassTrafficVehicleLightsFragment>();
	
		const int32 NumEntities = QueryContext.GetNumEntities();
		for(int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FMassEntityHandle EntityHandle = QueryContext.GetEntity(EntityIndex);

			// only check if the vehicle IS not an emergency vehicle
			const bool IsEMVehicle = EMVehicles.Contains(EntityHandle);
			if (!IsEMVehicle)
			{
				const FTransformFragment& TransformFragment = TransformFragments[EntityIndex];
				const FTransform& EntityTransform = TransformFragment.GetTransform();
				const FVector EntityFwd = EntityTransform.GetRotation().RotateVector(FVector::ForwardVector);

				// find nearest emergency vehicle that can make this entity to get out of the way
				double SqMinDist = EMRecognitionDistanceSquared;

				FMassEntityHandle NearestEMVehicle;
				for(int32 EMIndex = 0; EMIndex < NumEMVehicles; ++EMIndex)
				{
					const FMassEntityHandle EMHandle = EMVehicles[EMIndex];
				
					const FTransformFragment& EMTransformFragment = EntityManager.GetFragmentDataChecked<FTransformFragment>(EMHandle);
					const FTransform& EMTransform = EMTransformFragment.GetTransform();
					const FVector EMForward = EMTransform.GetRotation().RotateVector(FVector::ForwardVector);

					if (FVector::DotProduct(EntityFwd, EMForward) > 0.707f)
					{
						const FVector EMPosition = EMTransform.GetLocation();
						const FVector EntityPosition = EntityTransform.GetLocation();

						const double SqDist = FVector::DistSquared(EMPosition, EntityPosition); 
						if (SqDist < SqMinDist)
						{
							SqMinDist = SqDist;
							NearestEMVehicle = EMHandle; 
						}
					}
				}
				
				FMassTrafficVehicleControlFragment& VehicleControlFragment = VehicleControlFragments[EntityIndex];
				FMassTrafficVehicleLightsFragment& VehicleLightsFragment = VehicleLightsFragments[EntityIndex];
				
				if (!NearestEMVehicle.IsValid())
				{
					if (VehicleControlFragment.EmergencyOffset != 0)
					{
						VehicleControlFragment.EmergencyOffset = 0.0f;
						VehicleLightsFragment.bLeftTurnSignalLights = false;
						VehicleLightsFragment.bRightTurnSignalLights = false;
					}
				}
				else
				{
					const FMassZoneGraphLaneLocationFragment& LaneLocationFragment = LaneLocationFragments[EntityIndex];
					const FZoneGraphTrafficLaneData* EntityLane = MassTrafficSubsystem.GetTrafficLaneData(LaneLocationFragment.LaneHandle);
					if (EntityLane != nullptr && !EntityLane->bIsRightMostLane)
					{
						VehicleControlFragment.EmergencyOffset = -MassTrafficSettings->RescueLaneEvasion; 
					}
					else
					{
						VehicleControlFragment.EmergencyOffset = MassTrafficSettings->RescueLaneEvasion;
					}

					VehicleLightsFragment.bLeftTurnSignalLights = true;
					VehicleLightsFragment.bRightTurnSignalLights = true;
				}
			}
		}
	});
}
