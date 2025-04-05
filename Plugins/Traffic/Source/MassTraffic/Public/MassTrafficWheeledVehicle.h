// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassActorPoolableInterface.h"
#include "MassTrafficVehicleControlInterface.h"
#include "WheeledVehiclePawn.h"
#include "MassTrafficWheeledVehicle.generated.h"


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

protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Motion Blur")
	float BlurAngleVelocityMax = 3000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Motion Blur")
	float BlurAngleMax = 0.035f;

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	virtual void BeginPlay() override;

private:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> CachedMotionBlurWheelMIDs;
	TArray<float> CachedMotionBlurWheelAngle;
};
