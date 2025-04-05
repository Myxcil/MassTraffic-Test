// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassEntityTraitBase.h"
#include "MassObserverProcessor.h"
#include "MassTrafficLights.h"
#include "MassVisualizationTrait.h"

#include "MassTrafficLightVisualizationTrait.generated.h"

UCLASS(meta=(DisplayName="Traffic Light Visualization"))
class MASSTRAFFIC_API UMassTrafficLightVisualizationTrait : public UMassVisualizationTrait
{
	GENERATED_BODY()
	
public:
	
	UMassTrafficLightVisualizationTrait();

	UPROPERTY(EditAnywhere, Category = "Mass Traffic|Traffic Lights")
	FMassTrafficLightsParameters TrafficLightsParams;

public:

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	virtual void SanitizeParams(FMassRepresentationParameters& InOutParams, const bool bStaticMeshDeterminedInvalid = false) const override;

#if WITH_EDITOR
	virtual bool ValidateParams() const override;
#endif // WITH_EDITOR
};
