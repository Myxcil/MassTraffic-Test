// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassAgentTraits.h"
#include "MassCommonFragments.h"
#include "MassTrafficVehicleSyncTrait.generated.h"

class UChaosVehicleMovementComponent;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
USTRUCT()
struct FChaosVehicleMovementCopyToMassTag : public FMassTag
{
	GENERATED_BODY()
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
USTRUCT()
struct FChaosVehicleMovementComponentWrapperFragment : public FObjectWrapperFragment
{
	GENERATED_BODY()
	TWeakObjectPtr<UChaosVehicleMovementComponent> Component;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// SyncTraits
//------------------------------------------------------------------------------------------------------------------------------------------------------------
UCLASS(Abstract)
class MASSTRAFFIC_API UMassTrafficVehicleSyncTrait : public UMassAgentSyncTrait
{
	GENERATED_BODY()
};

UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "Vehicle Movement Sync"))
class MASSTRAFFIC_API UMassTrafficVehicleMovementSyncTrait : public UMassTrafficVehicleSyncTrait
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "Vehicle Orientation Sync"))
class MASSTRAFFIC_API UMassTrafficVehicleOrientationSyncTrait : public UMassAgentSyncTrait
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Translators
//------------------------------------------------------------------------------------------------------------------------------------------------------------
UCLASS()
class MASSTRAFFIC_API UMassTrafficVehicleMovementToMassTranslator : public UMassTranslator
{
	GENERATED_BODY()

public:
	UMassTrafficVehicleMovementToMassTranslator();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UCLASS()
class MASSTRAFFIC_API UMassTrafficVehicleOrientationToMassTranslator : public UMassTranslator
{
	GENERATED_BODY()

public:
	UMassTrafficVehicleOrientationToMassTranslator();
	
protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
