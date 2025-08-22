// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassTrafficFragments.h"
#include "MassTrafficProcessorBase.h"
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassCommonFragments.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassProcessor.h"
#include "MassTrafficInitIntersectionsProcessor.generated.h"


USTRUCT()
struct MASSTRAFFIC_API FMassTrafficIntersectionsSpawnData
{
	GENERATED_BODY()

	TArray<FMassTrafficIntersectionFragment> IntersectionFragments;
	TArray<FTransform> IntersectionTransforms;
};

UCLASS()
class MASSTRAFFIC_API UMassTrafficInitIntersectionsProcessor : public UMassTrafficProcessorBase
{
	GENERATED_BODY()

public:
	UMassTrafficInitIntersectionsProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
