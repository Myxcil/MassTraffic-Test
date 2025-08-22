// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassTrafficProcessorBase.h"
#include "MassTrafficFragments.h"
#include "MassTrafficPostPhysicsUpdateTrafficVehiclesProcessor.generated.h"


UCLASS()
class MASSTRAFFIC_API UMassTrafficPostPhysicsUpdateTrafficVehiclesProcessor : public UMassTrafficProcessorBase
{
	GENERATED_BODY()

public:
	UMassTrafficPostPhysicsUpdateTrafficVehiclesProcessor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery PIDControlTrafficVehicleQuery;
};
