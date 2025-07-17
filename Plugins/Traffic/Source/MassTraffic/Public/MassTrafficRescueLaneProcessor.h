// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "MassTrafficProcessorBase.h"
#include "MassTrafficFragments.h"
#include "MassActorSubsystem.h"
#include "MassTrafficRescueLaneProcessor.generated.h"

UCLASS()
class MASSTRAFFIC_API UMassTrafficRescueLaneProcessor : public UMassTrafficProcessorBase
{
	GENERATED_BODY()
	
public:
	UMassTrafficRescueLaneProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EMVehicleQuery;
	FMassEntityQuery VehicleQuery;
};
