// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "CoreMinimal.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassEntitySpawnDataGeneratorBase.h"
#include "MassEntityQuery.h"
#include "MassTrafficTrailerSpawnDataGenerator.generated.h"

/**
 * Searches all currently spawned vehicles with UMassTrafficConstrainedTrailerTrait and spawns their
 * TrailerAgentConfigAsset, constrained to each vehicle respectively.
 * 
 * Note: Only TrailerAgentConfigAsset's that appear in this spawners entity types list will be spawned. 
 */
UCLASS()
class MASSTRAFFIC_API UMassTrafficTrailerSpawnDataGenerator : public UMassEntitySpawnDataGeneratorBase
{
	GENERATED_BODY()

public:

	/** Generate "Count" number of SpawnPoints and return as a list of position
	 * @param Count of point to generate
	 * @param FinishedGeneratingSpawnPointsDelegate is the callback to call once the generation is done
	 */
	virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override;

protected:
	void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) const;

	mutable FMassEntityQuery VehicleQuery;
};
