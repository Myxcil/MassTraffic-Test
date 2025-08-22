// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassTrafficSettings.h"
#include "CoreMinimal.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassEntitySpawnDataGeneratorBase.h"
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "ZoneGraphTypes.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassTrafficParkedVehicleSpawnDataGenerator.generated.h"

UCLASS()
class MASSTRAFFIC_API UMassTrafficParkedVehicleSpawnDataGenerator : public UMassEntitySpawnDataGeneratorBase
{
	GENERATED_BODY()

public:

	/** The parking space type each entity can spawn in */
	UPROPERTY(EditAnywhere)
	TMap<TSoftObjectPtr<UMassEntityConfigAsset>, FName> EntityTypeToParkingSpaceType;

	UPROPERTY(EditAnywhere)
	FName DefaultParkingSpaceType;

	/**
	 * If true, the input Count is ignored and vehicles will be spawned in every parking space in ParkingSpaces.
	 * 
	 * Note: MassTraffic.NumParkedVehiclesScale is still applied. 
	 */  
	UPROPERTY(EditAnywhere)
	bool bUseAllParkingSpaces = false;

	/**
	 * All parking spaces within this distance to an 'obstacle' e.g the player or deviated vehicles, will be skipped.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ObstacleExclusionRadius = 500.0f;

	/** Generate "Count" number of SpawnPoints and return as a list of position
	 * @param Count of point to generate
	 * @param FinishedGeneratingSpawnPointsDelegate is the callback to call once the generation is done
	 */
	virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override;
};
