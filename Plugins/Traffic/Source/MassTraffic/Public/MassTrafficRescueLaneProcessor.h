// Fill out your copyright notice in the Description page of Project Settings.

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
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EMVehicleQuery;
	FMassEntityQuery VehicleQuery;
};
