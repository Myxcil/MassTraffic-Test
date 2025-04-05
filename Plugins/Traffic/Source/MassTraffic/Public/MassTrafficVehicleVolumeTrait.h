// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassTrafficFragments.h"
#include "MassTrafficVehicleVolumeTrait.generated.h"

USTRUCT()
struct MASSTRAFFIC_API FMassTrafficVehicleVolumeParameters : public FMassConstSharedFragment
{
	GENERATED_BODY()
	
	/**
	 * Distance from vehicle origin to front most point or rear most point (whichever is greater). Used for vehicle
	 * avoidance collision detection.
	 */  
	UPROPERTY(EditAnywhere, Category = "Dimensions")
	float HalfLength = 0.0f;
	
	/**
	* Distance from vehicle origin to left most point or right most point (whichever is greater). Used for vehicle
	* avoidance collision detection.
	*/  
	UPROPERTY(EditAnywhere, Category = "Dimensions")
	float HalfWidth = 0.0f;
};

UCLASS(meta=(DisplayName="Traffic Vehicle Volume Dimension"))
class MASSTRAFFIC_API UMassTrafficVehicleVolumeTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	UMassTrafficVehicleVolumeTrait(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, Category = "Mass Traffic")
	FMassTrafficVehicleVolumeParameters Params;

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};


