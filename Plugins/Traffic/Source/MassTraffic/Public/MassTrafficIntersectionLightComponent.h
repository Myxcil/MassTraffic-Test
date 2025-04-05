// (c) 2024 by Crenetic GmbH Studios

#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MassTrafficIntersectionLightComponent.generated.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASSTRAFFIC_API UMassTrafficIntersectionLightComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UMassTrafficIntersectionLightComponent();
};
