// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassActorPoolableInterface.h"
#include "GameFramework/Actor.h"
#include "MassTrafficBaseVehicle.generated.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
class UMassTrafficVehicleComponent;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Basic vehicle used for AI low-res representation in MassTraffic
UCLASS(Blueprintable)
class MASSTRAFFIC_API AMassTrafficBaseVehicle : public AActor, public IMassActorPoolableInterface
{
	GENERATED_BODY()

public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	AMassTrafficBaseVehicle(const FObjectInitializer& ObjectInitializer);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	// IMassActorPoolableInterface interface
	virtual bool CanBePooled_Implementation() override { return true; }
	virtual void PrepareForPooling_Implementation() override;
	virtual void PrepareForGame_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void ApplyWheelMotionBlurNative(const TArray<UMaterialInstanceDynamic*>& MotionBlurMIDs);

protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Motion Blur")
	float BlurAngleVelocityMax = 3000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Motion Blur")
	float BlurAngleMax = 0.035f;
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMassTrafficVehicleComponent> MassTrafficVehicleComponent;

	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> CachedMotionBlurWheelMIDs;
	TArray<float> CachedMotionBlurWheelAngle;
};
