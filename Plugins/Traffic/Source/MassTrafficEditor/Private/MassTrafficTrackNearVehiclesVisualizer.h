// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
class MASSTRAFFICEDITOR_API FMassTrafficTrackNearVehiclesVisualizer : public FComponentVisualizer
{
public:
	FMassTrafficTrackNearVehiclesVisualizer();
	virtual ~FMassTrafficTrackNearVehiclesVisualizer() override;

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};

