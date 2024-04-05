// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassTrafficProxyActor.h"
#include "MassTrafficLightActor.generated.h"

UCLASS()
class MASSTRAFFIC_API AMassTrafficLightActor : public AMassTrafficProxyActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int16 TrafficLightTypeIndex = INDEX_NONE;
	
public:
	int16 GetTrafficLightTypeIndex() const { return TrafficLightTypeIndex; }
};
