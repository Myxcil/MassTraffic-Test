// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "MassTrafficWheeledVehicle.h"
#include "MassTrafficControlledVehicle.generated.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
struct FInputActionValue;
class AAIController;
class UInputAction;
class UInputMappingContext;
class UMassTrafficTrackNearVehicles;
class UMassTrafficPathFinder;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Basic vehicle class used for AI or player controlled vehicles
// that can also interact with the MassTraffic system
// Not placed like normal actors but spawned via MassSpawner and AgentConfig
UCLASS(Blueprintable)
class MASSTRAFFIC_API AMassTrafficControlledVehicle : public AMassTrafficWheeledVehicle
{
	GENERATED_BODY()

public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	explicit AMassTrafficControlledVehicle(const FObjectInitializer& ObjectInitializer);
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void DetachFromControllerPendingDestroy() override;
	virtual void Destroyed() override;

	virtual void Tick(float DeltaSeconds) override;

	//~ Begin IMassActorPoolableInterface
	virtual bool CanBePooled_Implementation() override { return false; }
	//~ End IMassActorPoolableInterface

	UMassTrafficPathFinder* GetPathFinder() const { return PathFinder; }
	UMassTrafficTrackNearVehicles* GetNearVehicleTracker() const { return NearVehicleTracker; }
	float GetAgentRadius() const { return AgentRadius; }
	float GetNoiseInput() const { return NoiseInput; }

	float GetSpeed() const;
	virtual bool HasStopped() const;

	virtual void ResetVehicle();

protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMassTrafficPathFinder> PathFinder;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMassTrafficTrackNearVehicles> NearVehicleTracker;

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	virtual void BeginPlay() override;

	void SetThrottle(const FInputActionValue& Value);
	void SetBrake(const FInputActionValue& Value);
	void SetSteering(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	float AgentRadius = 100.0f;
	float NoiseInput = 0.0f;
	bool bIsDestroyed = false;
	TObjectPtr<AAIController> OriginalAIControlller;
	FTransform SpawnTransform;
};
