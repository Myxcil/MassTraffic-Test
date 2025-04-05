// (c) 2024 by Crenetic GmbH Studios

#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "CoreMinimal.h"
#include "AIController.h"
#include "MassTrafficPIDController.h"
#include "ZoneGraphTypes.h"
#include "MassTrafficAIDriver.generated.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
class UMassTrafficIntersectionComponent;
class UMassEntitySubsystem;
class UMassTrafficSettings;
class AMassTrafficControlledVehicle;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
DECLARE_LOG_CATEGORY_EXTERN(LogAIDriver, Log, All);

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UENUM()
enum class EDrivingState : uint8
{
	Stopped,
	FollowingPath,
	FreeDrive,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
DECLARE_DYNAMIC_DELEGATE(FOnPathFinished);

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// AI controller class for handling the MassTrafficControlledVehicle
// It uses data from the MassTraffic simulation but uses it's own control scheme
// ControlledVehicle will be treated like a PlayerVehicleTag object 
UCLASS()
class MASSTRAFFIC_API AMassTrafficAIDriver : public AAIController
{
	GENERATED_BODY()

public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	AMassTrafficAIDriver();

	virtual void Tick(float DeltaSeconds) override;
	
	bool SetDestination(const FVector& Location, const FOnPathFinished& PathFinished);
	
	UFUNCTION(CallInEditor, BlueprintCallable)
	void SetEmergencyMode(const bool bEnabled);
	UFUNCTION(BlueprintPure)
	bool IsEmergencyMode() const { return bIsEmergencyMode; };
	
protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly)
	float LookAheadDistance = 200.0f;
	UPROPERTY(EditDefaultsOnly)
	float MaxSteeringAngle = 50.0f;
	UPROPERTY(EditDefaultsOnly)
	float ThrottleScale = 1.0f;
	UPROPERTY(EditDefaultsOnly)
	FMassTrafficPIDControllerParams SteeringPIDParameter; 
	UPROPERTY(EditDefaultsOnly)
	FMassTrafficPIDControllerParams ThrottlePIDParameter;

private:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	void HandlePathFollowing(const float DeltaSeconds);
	float QuerySteeringInformationFromPathFinder(const FTransform& Transform);
	void ResetPriorityLaneOnIntersection();

	void OnLaneChange(const FZoneGraphLaneHandle& OldLane, const FZoneGraphLaneHandle& NewLane);
	
private:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	TWeakObjectPtr<const UMassTrafficSettings> MassTrafficSettings;
	TObjectPtr<UMassEntitySubsystem> EntitySubsystem;

	UPROPERTY(Transient, VisibleAnywhere, Category="Vehicle")
	AMassTrafficControlledVehicle* ControlledVehicle = nullptr;
	float RandomFraction = 0.0f;

	EDrivingState DrivingState = EDrivingState::Stopped;
	FVector SteeringTargetPosition = FVector::ForwardVector;
	FQuat SteeringTargetOrientation = FQuat::Identity;
	FMassTrafficPIDController SteeringController;
	FMassTrafficPIDController ThrottleController;

	bool bIsEmergencyMode = false;
	
	FZoneGraphLaneHandle IntersectionLaneHandle;
	TWeakObjectPtr<UMassTrafficIntersectionComponent> PriorityIntersection;

	FOnPathFinished OnPathFinished;
};
