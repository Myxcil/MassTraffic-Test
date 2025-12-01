// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassTrafficPhysics.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassEntityTraitBase.h"
#include "MassSimulationLOD.h"
#include "WheeledVehiclePawn.h"
#include "MassTrafficVehicleSimulationTrait.generated.h"

USTRUCT()
struct MASSTRAFFIC_API FMassTrafficVehicleSimulationParameters : public FMassConstSharedFragment
{
	GENERATED_BODY()
	
	/** Distance along X from vehicle origin to front axel (i.e front half of wheelbase) */
	UPROPERTY(EditAnywhere, Category = "Dimensions")
	float FrontAxleX = 150;
	
	/** Negative distance along X from vehicle origin to rear axel (i.e back half of wheelbase) */
	UPROPERTY(EditAnywhere, Category = "Dimensions")
	float RearAxleX = -150;

	/**
	 * If true, this vehicle will only be allowed to drive on lanes matching AMassTrafficCoordinator::TrunkLaneFilter
	 * e.g: to restrict large vehicles to freeways
	 */
	UPROPERTY(EditAnywhere, Category = "Restrictions")
	bool bRestrictedToTrunkLanesOnly = false;

	/** Actor class of this agent when spawned in high resolution */
	UPROPERTY(EditAnywhere, Category = "Physics")
	TSubclassOf<AWheeledVehiclePawn> PhysicsVehicleTemplateActor;
};

UCLASS(meta=(DisplayName="Traffic Vehicle Simulation"))
class MASSTRAFFIC_API UMassTrafficVehicleSimulationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:

	UMassTrafficVehicleSimulationTrait(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, Category = "Mass Traffic")
	FMassTrafficVehicleSimulationParameters Params;

	UPROPERTY(EditAnywhere, Category = "Variable Tick")
	FMassSimulationVariableTickParameters VariableTickParams;

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
