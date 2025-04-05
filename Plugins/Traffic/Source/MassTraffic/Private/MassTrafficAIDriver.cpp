// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficAIDriver.h"

#include "MassAgentComponent.h"
#include "MassEntitySubsystem.h"
#include "MassTrafficControlledVehicle.h"
#include "MassTrafficFragments.h"
#include "MassTrafficIntersectionComponent.h"
#include "MassTrafficMovement.h"
#include "MassTrafficPathFinder.h"
#include "MassTrafficSettings.h"
#include "MassTrafficTrackNearVehicles.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_LOG_CATEGORY(LogAIDriver);
#define DRIVE_LOG(Format, ...) UE_PRIVATE_LOG(PREPROCESSOR_NOTHING, constexpr, LogAIDriver, Log, Format, ##__VA_ARGS__)

//------------------------------------------------------------------------------------------------------------------------------------------------------------
AMassTrafficAIDriver::AMassTrafficAIDriver()
{
	PrimaryActorTick.bCanEverTick = true;
	RandomFraction = FMath::Frac(FMath::FRand());
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficAIDriver::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!ControlledVehicle)
		return;

	switch (DrivingState)
	{
	case EDrivingState::Stopped:
		ControlledVehicle->GetVehicleMovement()->bReverseAsBrake = false;
		IMassTrafficVehicleControlInterface::Execute_SetVehicleInputs(ControlledVehicle, 0, 1, true, 0, true);
		break;
		
	case EDrivingState::FollowingPath:
		HandlePathFollowing(DeltaSeconds);
		break;

	case EDrivingState::FreeDrive:
		break;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficAIDriver::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AMassTrafficControlledVehicle* Vehicle = Cast<AMassTrafficControlledVehicle>(InPawn))
	{
		if (ControlledVehicle == Vehicle)
			return;
	
		DrivingState = EDrivingState::Stopped;

		ControlledVehicle = Vehicle;

		if (ControlledVehicle)
		{
			MassTrafficSettings = GetDefault<UMassTrafficSettings>();
			EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
			ControlledVehicle->GetVehicleMovement()->bReverseAsBrake = false;

			if (UMassTrafficPathFinder* PathFinder = ControlledVehicle->GetPathFinder())
			{
				PathFinder->OnLaneChanged.BindUObject(this, &ThisClass::OnLaneChange);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficAIDriver::OnUnPossess()
{
	if (ControlledVehicle)
	{
		if (UMassTrafficPathFinder* PathFinder = ControlledVehicle->GetPathFinder())
		{
			PathFinder->OnLaneChanged.BindUObject(this, &ThisClass::OnLaneChange);
		}
		ControlledVehicle = nullptr;
	}
	Super::OnUnPossess();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficAIDriver::HandlePathFollowing(const float DeltaSeconds)
{
	// get current state of the vehicle like world position and lane information from ZoneGraph
	const FTransform& Transform = ControlledVehicle->GetTransform();
	const UMassTrafficPathFinder* PathFinder = ControlledVehicle->GetPathFinder();
	const FZoneGraphTrafficLaneData* CurrLane = PathFinder->GetCurrentLane();
	if (CurrLane == nullptr)
		return;
	
	const FZoneGraphLaneLocation& CurrLocation = PathFinder->GetCurrentLocation();

	const float TargetSteering = QuerySteeringInformationFromPathFinder(Transform);
	
	const FVector Velocity = ControlledVehicle->GetVelocity();
	const float Speed = Velocity.Length() * FMath::Sign(FVector::DotProduct(Velocity, Transform.GetUnitAxis(EAxis::X)));

	bool bRequestDifferentNextLane;
	bool bVehicleCantStopAtLaneExit = false;
	bool bIsFrontOfVehicleBeyondEndOfLane;
	bool bVehicleHasNoNextLane;
	bool bVehicleHasNoRoom;

	const bool bMustStopAtLaneExit = CurrLane ? UE::MassTraffic::ShouldStopAtLaneExit(
		CurrLocation.DistanceAlongLane,
		Speed,
		ControlledVehicle->GetAgentRadius(),
		RandomFraction,
		CurrLane->Length,
		PathFinder->GetNextLane(),
		MassTrafficSettings->MinimumDistanceToNextVehicleRange,
		EntitySubsystem->GetEntityManager(),
		/*out*/bRequestDifferentNextLane,
		/*in/out*/bVehicleCantStopAtLaneExit,
		/*out*/bIsFrontOfVehicleBeyondEndOfLane,
		/*out*/bVehicleHasNoNextLane,
		/*out*/bVehicleHasNoRoom,
		MassTrafficSettings->StandardTrafficPrepareToStopSeconds
	) : false;

	// Check for vehicle in front and adjust speed accordingly
	const FNearestVehicleInfo& NearestVehicleInfo = ControlledVehicle->GetNearVehicleTracker()->GetNearestVehicleInfo();

	// Calculate new target speed depending on current lane speed limit
	const float SpeedMultiplier = bIsEmergencyMode ? MassTrafficSettings->RescueLaneEMSpeedMultiplier : 1.0f;
	const float LaneMaxSpeed = CurrLane->ConstData.SpeedLimit * SpeedMultiplier;
	const float NextLaneMaxSpeed = CurrLane->ConstData.AverageNextLanesSpeedLimit * SpeedMultiplier;
	const float SpeedLimit = UE::MassTraffic::GetSpeedLimitAlongLane(CurrLane->Length,LaneMaxSpeed, NextLaneMaxSpeed, CurrLocation.DistanceAlongLane, Speed, MassTrafficSettings->SpeedLimitBlendTime); 

	// Compute stable distance based noise
	const float NoiseValue = UE::MassTraffic::CalculateNoiseValue(ControlledVehicle->GetNoiseInput(), MassTrafficSettings->NoisePeriod);
	const float VariedSpeedLimit = UE::MassTraffic::VarySpeedLimit(SpeedLimit, MassTrafficSettings->SpeedLimitVariancePct, MassTrafficSettings->SpeedVariancePct, RandomFraction, NoiseValue);

	const float CurrLaneLength = PathFinder->UpdateLaneLength(CurrLane);

	float TargetSpeed = CurrLane ? UE::MassTraffic::CalculateTargetSpeed(
			CurrLocation.DistanceAlongLane,
			Speed,
			NearestVehicleInfo.Distance,
			NearestVehicleInfo.TimeToCollision,
			NearestVehicleInfo.DistanceToCollision,
			ControlledVehicle->GetAgentRadius(),
			RandomFraction,
			CurrLaneLength,
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
		) : VariedSpeedLimit;

	// reduce TargetSpeed if we are in a curve
	const float TurnAngle = Transform.InverseTransformVectorNoScale(SteeringTargetOrientation.GetForwardVector()).HeadingAngle();
	const float TurnSpeedFactor = FMath::GetMappedRangeValueClamped<>(TRange<float>(0.0f, HALF_PI), TRange<float>(1.0f, MassTrafficSettings->TurnSpeedScale), FMath::Abs(TurnAngle));
	TargetSpeed *= TurnSpeedFactor;

	// Update the PID controllers for throttle and steering and send the inputs
	// to the vehicle control interface
	const float ThrottleAndBrake = ThrottleController.Tick(TargetSpeed , Speed, DeltaSeconds, ThrottlePIDParameter);
	const float Steering = SteeringController.Tick(0, -TargetSteering, DeltaSeconds, SteeringPIDParameter);
	
	const float Throttle = FMath::Max(0, ThrottleAndBrake) * ThrottleScale;
	const float Brake = 5.0f * -FMath::Min(0, ThrottleAndBrake);
	
	IMassTrafficVehicleControlInterface::Execute_SetVehicleInputs(ControlledVehicle, Throttle, Brake, false, Steering, Steering != 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool AMassTrafficAIDriver::SetDestination(const FVector& Location, const FOnPathFinished& PathFinished)
{
	UMassTrafficPathFinder* PathFinder = ControlledVehicle->GetPathFinder(); 
	if (PathFinder->SearchPath(Location))
	{
		DrivingState = EDrivingState::FollowingPath;
		OnPathFinished = PathFinished;

		ControlledVehicle->GetVehicleMovement()->bReverseAsBrake = true;
		PathFinder->InitPathFollowing();
		DRIVE_LOG(TEXT("%s SetDestination %s success"), *ControlledVehicle->GetName(), *Location.ToString());
		
		return true;
	}

	DrivingState = EDrivingState::Stopped;
	ControlledVehicle->GetVehicleMovement()->bReverseAsBrake = false;
	DRIVE_LOG(TEXT("%s SetDestination %s failed"), *ControlledVehicle->GetName(), *Location.ToString());

	return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficAIDriver::SetEmergencyMode(const bool bEnabled)
{
	if (bIsEmergencyMode == bEnabled)
		return;
	
	bIsEmergencyMode = bEnabled;

	ResetPriorityLaneOnIntersection();

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	if (const UMassAgentComponent* AgentComponent = ControlledVehicle->GetComponentByClass<UMassAgentComponent>())
	{
		const FMassEntityHandle AgentHandle = AgentComponent->GetEntityHandle();
		if (AgentHandle.IsValid())
		{
			if (bIsEmergencyMode)
			{
				EntityManager.AddTagToEntity(AgentHandle, FMassTrafficEmergencyTag::StaticStruct());
			}
			else
			{
				EntityManager.RemoveTagFromEntity(AgentHandle, FMassTrafficEmergencyTag::StaticStruct());
			}
		}
	}
}
	
//------------------------------------------------------------------------------------------------------------------------------------------------------------
float AMassTrafficAIDriver::QuerySteeringInformationFromPathFinder(const FTransform& Transform)
{
	UMassTrafficPathFinder* PathFinder = ControlledVehicle->GetPathFinder();
	if (!PathFinder->UpdatePathFollowing(LookAheadDistance, SteeringTargetPosition, SteeringTargetOrientation))
	{
		DrivingState = EDrivingState::Stopped;
		ControlledVehicle->GetVehicleMovement()->bReverseAsBrake = false;
		DRIVE_LOG(TEXT("%s stoppped pathfinding at %s"), *ControlledVehicle->GetName(), *ControlledVehicle->GetActorLocation().ToString());
		if (OnPathFinished.IsBound())
		{
			OnPathFinished.Execute();
			OnPathFinished.Unbind();
		}
		return 0;
	}

	if (bIsEmergencyMode)
	{
		const FVector EmergencyOffset = -SteeringTargetOrientation.GetRightVector() * MassTrafficSettings->RescueLaneMaxEvasion;
		SteeringTargetPosition += EmergencyOffset;
	}

	FVector SteeringDirection(Transform.InverseTransformPositionNoScale(SteeringTargetPosition));
	SteeringDirection.Z = 0;
	const float RelativeSteering = FMath::RadiansToDegrees(SteeringDirection.HeadingAngle()) / MaxSteeringAngle;

	return RelativeSteering;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficAIDriver::ResetPriorityLaneOnIntersection()
{
	if (UMassTrafficIntersectionComponent* IntersectionComponent = PriorityIntersection.Get())
	{
		DRIVE_LOG(TEXT("Priority reset for intersection #%d"), IntersectionLaneHandle.Index);
		IntersectionComponent->SetEmergencyLane(IntersectionLaneHandle, false);
		PriorityIntersection.Reset();
		IntersectionLaneHandle.Reset();
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficAIDriver::OnLaneChange(const FZoneGraphLaneHandle& OldLane, const FZoneGraphLaneHandle& NewLane)
{
	// a) both lanes valid, middle of the path
	// b) old lane invalid? Here we start our journey
	// c) new lane invalid? We have come to an end
	UMassTrafficPathFinder* PathFinder = ControlledVehicle->GetPathFinder();

	if (bIsEmergencyMode)
	{
		if (OldLane.IsValid())
		{
			PathFinder->SetEmergencyLane(OldLane, false);
		}
		if (NewLane.IsValid())
		{
			PathFinder->SetEmergencyLane(NewLane, true);
			if (const FZoneGraphTrafficLaneData* NextLaneData = PathFinder->GetNextLane())
			{
				PathFinder->SetEmergencyLane(NextLaneData->LaneHandle, true);
			}
		}
	}

	if (OldLane == IntersectionLaneHandle)
	{
		ResetPriorityLaneOnIntersection();
	}

	if (bIsEmergencyMode && NewLane.IsValid() && !IntersectionLaneHandle.IsValid())
	{
		if (const FZoneGraphTrafficLaneData* NextLaneData = PathFinder->GetNextLane())
		{
			if (UMassTrafficIntersectionComponent* IntersectionComponent = UMassTrafficIntersectionComponent::FindIntersection(NextLaneData->LaneHandle))
			{
				IntersectionComponent->SetEmergencyLane(NextLaneData->LaneHandle, true);
				PriorityIntersection = IntersectionComponent;
				IntersectionLaneHandle = NextLaneData->LaneHandle;
				DRIVE_LOG(TEXT("Priority set for intersection #%d"), IntersectionLaneHandle.Index);
				return;
			}
		}
	}
}

