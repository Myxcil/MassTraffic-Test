// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassTrafficProcessorBase.h"
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassRepresentationFragments.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassProcessor.h"
#include "MassTrafficUpdateIntersectionsProcessor.generated.h"


UCLASS()
class MASSTRAFFIC_API UMassTrafficUpdateIntersectionsProcessor : public UMassTrafficProcessorBase
{
	GENERATED_BODY()

protected:
	UMassTrafficUpdateIntersectionsProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
