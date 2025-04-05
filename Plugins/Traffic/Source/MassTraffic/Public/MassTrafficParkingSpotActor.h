// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassTrafficProxyActor.h"
#include "MassTrafficParkingSpotActor.generated.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
class UMassTrafficParkingSpotComponent;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UCLASS()
class MASSTRAFFIC_API AMassTrafficParkingSpotActor : public AMassTrafficProxyActor
{
	GENERATED_BODY()

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess=true))
	FName ParkingSpaceType;

public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	explicit AMassTrafficParkingSpotActor(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable)
	const FName& GetParkingSpaceType() const { return ParkingSpaceType; }

private:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY()
	TObjectPtr<UMassTrafficParkingSpotComponent> ParkingSpotComponent;
};
