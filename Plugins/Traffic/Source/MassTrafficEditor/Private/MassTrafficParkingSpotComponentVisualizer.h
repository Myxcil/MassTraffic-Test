// Copyright 2024 Crenetic GmbH Studios All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
class MASSTRAFFICEDITOR_API FMassTrafficParkingSpotComponentVisualizer : public FComponentVisualizer
{
public:
	FMassTrafficParkingSpotComponentVisualizer();
	virtual ~FMassTrafficParkingSpotComponentVisualizer() override;

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
