// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficTrackNearVehicles.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassTrafficControlledVehicle.h"
#include "MassTrafficFragments.h"
#include "MassTrafficMovement.h"
#include "MassTrafficPathFollower.h"
#include "MassTrafficTypes.h"
#include "MassZoneGraphNavigationFragments.h"


UMassTrafficTrackNearVehicles::UMassTrafficTrackNearVehicles(): NearestVehicleInfo()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMassTrafficTrackNearVehicles::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
		
	ControlledVehicle = Cast<AMassTrafficControlledVehicle>(GetOwner());

	EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySubsystem);
	MassActorSubsystem = World->GetSubsystem<UMassActorSubsystem>();
	check(MassActorSubsystem);
}

void UMassTrafficTrackNearVehicles::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DetermineNearestVehicle();
}

void UMassTrafficTrackNearVehicles::DetermineNearestVehicle()
{
	NearestVehicleInfo.Reset();

	UMassTrafficPathFollower* PathFollower = ControlledVehicle->GetPathFollower();
	check(PathFollower);
	
	const FZoneGraphLaneLocation& CurrLocation = PathFollower->GetCurrentLocation();
	if (CurrLocation.IsValid())
	{
		const FZoneGraphTrafficLaneData* CurrLane = PathFollower->GetCurrentLane();
		if (CurrLane && CurrLane->TailVehicle.IsSet())
		{
			const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();

			FMassEntityHandle NearestVehicleHandle;
			{
				FMassEntityHandle NextVehicle = CurrLane->TailVehicle;
				int32 MaxRuns = 50;
				float MinDist = TNumericLimits<float>::Max();
		
				while (NextVehicle.IsSet() && --MaxRuns > 0)
				{
					const FMassZoneGraphLaneLocationFragment* LaneLocationFragment = EntityManager.GetFragmentDataPtr<FMassZoneGraphLaneLocationFragment>(NextVehicle);
					const float DistanceToNextVehicle = LaneLocationFragment->DistanceAlongLane - CurrLocation.DistanceAlongLane;
					if (DistanceToNextVehicle > 0 && DistanceToNextVehicle < MinDist)
					{
						MinDist = DistanceToNextVehicle;
						NearestVehicleHandle = NextVehicle;
					}
	
					const FMassTrafficNextVehicleFragment* NextVehicleFragment = EntityManager.GetFragmentDataPtr<FMassTrafficNextVehicleFragment>(NextVehicle);
					check(NextVehicleFragment);
					if (NextVehicleFragment->HasNextVehicle())
					{
						NextVehicle = NextVehicleFragment->GetNextVehicle();
					}
					else
					{
						NextVehicle.Reset();
					}
				}
			}

			if (NearestVehicleHandle.IsSet())
			{
				const FMassVelocityFragment* VelocityFragment = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(NearestVehicleHandle);
				const FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(NearestVehicleHandle);
				const FMassZoneGraphLaneLocationFragment* LaneLocationFragment = EntityManager.GetFragmentDataPtr<FMassZoneGraphLaneLocationFragment>(NearestVehicleHandle);

				NearestVehicleInfo.Handle = NearestVehicleHandle;
				NearestVehicleInfo.Position = TransformFragment->GetTransform().GetLocation();
				NearestVehicleInfo.Speed = VelocityFragment->Value.Length();
				NearestVehicleInfo.Distance = LaneLocationFragment->DistanceAlongLane - CurrLocation.DistanceAlongLane;

				const FTransform& AgentTransform = ControlledVehicle->GetTransform();
				const FVector AgentVelocity = ControlledVehicle->GetVelocity();

				const FAgentRadiusFragment* AgentRadiusFragment = EntityManager.GetFragmentDataPtr<FAgentRadiusFragment>(NearestVehicleHandle);
				const float TimeToCollision = UE::MassTraffic::TimeToCollision(
						AgentTransform.GetLocation(), AgentVelocity, ControlledVehicle->GetAgentRadius(),
						NearestVehicleInfo.Position, VelocityFragment->Value, AgentRadiusFragment->Radius);

				if (TimeToCollision < TNumericLimits<float>::Max())
				{
					const FMassEntityHandle AgentHandle = MassActorSubsystem->GetEntityHandleFromActor(ControlledVehicle);
					const FMassTrafficVehicleVolumeParameters* ObstacleParams = EntityManager.GetConstSharedFragmentDataPtr<FMassTrafficVehicleVolumeParameters>(NearestVehicleHandle);
					if (!ObstacleParams || UE::MassTraffic::WillCollide(AgentTransform.GetLocation(), AgentTransform.GetRotation(), AgentVelocity, HalfWidth, HalfLength, NearestVehicleInfo.Position, TransformFragment->GetTransform().GetRotation(), VelocityFragment->Value, *ObstacleParams, TimeToCollision))
					{
						NearestVehicleInfo.TimeToCollision = TimeToCollision;
						NearestVehicleInfo.DistanceToCollision = FMath::Max(FVector::Distance(ControlledVehicle->GetActorLocation(), NearestVehicleInfo.Position) - AgentRadiusFragment->Radius - ControlledVehicle->GetAgentRadius(), 0.0f);
					}
				}
			}
		}
	}
}
