// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassTrafficProxyActor.h"
#include "MassTrafficParkingSpotActor.generated.h"

UCLASS()
class MASSTRAFFIC_API AMassTrafficParkingSpotActor : public AMassTrafficProxyActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName ParkingSpaceType;

public:
	UFUNCTION(BlueprintCallable)
	const FName& GetParkingSpaceType() const { return ParkingSpaceType; }
};
