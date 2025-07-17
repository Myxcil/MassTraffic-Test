// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficControlledVehicle.h"

#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MassTrafficPathFinder.h"
#include "MassTrafficTrackNearVehicles.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
AMassTrafficControlledVehicle::AMassTrafficControlledVehicle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PathFinder = CreateDefaultSubobject<UMassTrafficPathFinder>("PathFinder");
	NearVehicleTracker = CreateDefaultSubobject<UMassTrafficTrackNearVehicles>("NearVehicleTracker");

	UChaosVehicleMovementComponent* MovementComponent = GetVehicleMovement();
	check(MovementComponent);
	MovementComponent->bReverseAsBrake = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::BeginPlay()
{
	Super::BeginPlay();

	OriginalAIControlller = Cast<AAIController>(GetController());
	SpawnTransform = GetTransform();
	NoiseInput = FMath::Frac(FMath::FRand()) * 10000.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::UnPossessed()
{
	const bool bWasControlledByPlayer = Controller->IsA<APlayerController>();

	if (UChaosVehicleMovementComponent* MovementComponent = GetVehicleMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	Super::UnPossessed();

	if (bWasControlledByPlayer && OriginalAIControlller)
	{
		OriginalAIControlller->Possess(this);
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::DetachFromControllerPendingDestroy()
{
	if (bIsDestroyed)
	{
		Super::DetachFromControllerPendingDestroy();
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::Destroyed()
{
	bIsDestroyed = true;
	Super::Destroyed();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (PathFinder)
	{
		NoiseInput += GetVelocity().Length() * DeltaSeconds;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::SetThrottle(const FInputActionValue& Value)
{
	UChaosVehicleMovementComponent* MovementComponent = GetVehicleMovement();
	check(MovementComponent);
	MovementComponent->SetThrottleInput(Value.Get<float>());
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::SetBrake(const FInputActionValue& Value)
{
	UChaosVehicleMovementComponent* MovementComponent = GetVehicleMovement();
	check(MovementComponent);
	MovementComponent->SetBrakeInput(Value.Get<float>());
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::SetSteering(const FInputActionValue& Value)
{
	UChaosVehicleMovementComponent* MovementComponent = GetVehicleMovement();
	check(MovementComponent);
	MovementComponent->SetSteeringInput(Value.Get<float>());
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::HandleLook(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
float AMassTrafficControlledVehicle::GetSpeed() const
{
	return GetVehicleMovement()->GetForwardSpeed();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool AMassTrafficControlledVehicle::HasStopped() const
{
	const float AbsSpeed = FMath::Abs(GetVehicleMovement()->GetForwardSpeed());
	return AbsSpeed < 50.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficControlledVehicle::ResetVehicle()
{
	if (Controller)
	{
		Controller->StopMovement();
	}

	if (UChaosVehicleMovementComponent* MovementComponent = GetVehicleMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetThrottleInput(0);
		MovementComponent->SetSteeringInput(0);
		MovementComponent->SetHandbrakeInput(true);
	}
}
