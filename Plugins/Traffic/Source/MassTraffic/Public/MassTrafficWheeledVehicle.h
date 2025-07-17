// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassActorPoolableInterface.h"
#include "MassTrafficVehicleControlInterface.h"
#include "WheeledVehiclePawn.h"
#include "MassTrafficWheeledVehicle.generated.h"


class UChaosWheeledVehicleMovementComponent;
//------------------------------------------------------------------------------------------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FToggleMotorDelegate, bool, NewMotorState);

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UCLASS()
class MASSTRAFFIC_API AMassTrafficWheeledVehicle : public AWheeledVehiclePawn,
												public IMassActorPoolableInterface,
												public IMassTrafficVehicleControlInterface
{
	GENERATED_BODY()

public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	explicit AMassTrafficWheeledVehicle(const FObjectInitializer& ObjectInitializer);

	//~ Begin IMassActorPoolableInterface
	virtual bool CanBePooled_Implementation() override;
	virtual void PrepareForPooling_Implementation() override;
	virtual void PrepareForGame_Implementation() override;
	//~ End IMassActorPoolableInterface

	//~ Begin IMassTrafficVehicleControlInterface
	virtual void SetVehicleInputs_Implementation(const float Throttle, const float Brake, const bool bHandBrake, const float Steering, const bool bSetSteering) override;
	virtual void OnParkedVehicleSpawned_Implementation() override;
	virtual void OnTrafficVehicleSpawned_Implementation() override;
	//~ End IMassTrafficVehicleControlInterface 

	UFUNCTION(BlueprintCallable)
	void ApplyWheelMotionBlurParameters(const TArray<UMaterialInstanceDynamic*> MotionBlurMIDs);

	UFUNCTION(BlueprintPure)
	virtual bool IsMotorRunning() const { return true; }
	
protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Motion Blur")
	float BlurAngleVelocityMax = 3000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Motion Blur")
	float BlurAngleMax = 0.035f;

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleSound")
	USoundBase* EngineSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleSound")
	USoundBase* EngineStartSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleSound")
	USoundBase* EngineStopSound;
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	FToggleMotorDelegate OnToggleMotor;
	TWeakObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosMovementComponent;

private:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> CachedMotionBlurWheelMIDs;
	TArray<float> CachedMotionBlurWheelAngle;

	bool SoundEnabled = false;	

	TWeakObjectPtr<UAudioComponent> AudioEngine;

	float EngineStartDelay = 2.548f;
	float EngineLoopDelay = 1.807f;

	void HandleVehicleSound();
	void PlayEngineSound();
	UFUNCTION()
	void ToggleEngineSound(bool MotorState);
};
