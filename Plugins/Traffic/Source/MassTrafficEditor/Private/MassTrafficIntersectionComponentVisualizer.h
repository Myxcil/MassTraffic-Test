// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "UObject/Object.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
class MASSTRAFFICEDITOR_API FMassTrafficIntersectionComponentVisualizer : public FComponentVisualizer
{
public:
	FMassTrafficIntersectionComponentVisualizer();
	virtual ~FMassTrafficIntersectionComponentVisualizer() override;

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
