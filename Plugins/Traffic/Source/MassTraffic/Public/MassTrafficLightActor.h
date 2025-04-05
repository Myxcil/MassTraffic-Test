// (c) 2024 by Crenetic GmbH Studios

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
