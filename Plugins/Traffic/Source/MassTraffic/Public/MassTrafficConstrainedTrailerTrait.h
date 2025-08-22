// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassTrafficPhysics.h"
#include "CoreMinimal.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassEntityConfigAsset.h"
#include "Engine/DataTable.h"
#include "MassEntityTraitBase.h"
#include "MassSimulationLOD.h"
#include "MassTrafficConstrainedTrailerTrait.generated.h"

USTRUCT()
struct MASSTRAFFIC_API FMassTrafficConstrainedTrailerParameters : public FMassConstSharedFragment
{
	GENERATED_BODY()

	/**
	 * The trailer agent to spawn attached to this vehicle.
	 * @see UMassTrafficTrailerSpawnDataGenerator
	 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMassEntityConfigAsset> TrailerAgentConfigAsset = nullptr;
};

/**
 * In concert with UMassTrafficTrailerSpawnDataGenerator, spawns a TrailerAgentConfigAsset entity and point constrains
 * its vehicle simulation to this vehicle.
 */
UCLASS(meta=(DisplayName="Constrained Trailer"))
class MASSTRAFFIC_API UMassTrafficConstrainedTrailerTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Mass Traffic")
	FMassTrafficConstrainedTrailerParameters Params;

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
