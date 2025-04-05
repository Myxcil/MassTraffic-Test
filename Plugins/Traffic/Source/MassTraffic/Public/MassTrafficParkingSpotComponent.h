// Copyright 2024 Crenetic GmbH Studios All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MassTrafficParkingSpotComponent.generated.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
UCLASS()
class MASSTRAFFIC_API UMassTrafficParkingSpotComponent : public UActorComponent
{
	GENERATED_BODY()

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess=true))
	FVector2D Size = {500,250};
	
public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UMassTrafficParkingSpotComponent();

	const FVector2D& GetSize() const { return Size; }
};
