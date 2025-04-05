// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
class MASSTRAFFICEDITOR_API FMassTrafficPathFinderVisualizer : public FComponentVisualizer
{
public:
	FMassTrafficPathFinderVisualizer();
	virtual ~FMassTrafficPathFinderVisualizer() override;

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
