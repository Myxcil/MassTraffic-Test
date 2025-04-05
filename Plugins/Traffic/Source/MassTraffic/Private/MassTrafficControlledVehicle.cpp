// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficControlledVehicle.h"

#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
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
