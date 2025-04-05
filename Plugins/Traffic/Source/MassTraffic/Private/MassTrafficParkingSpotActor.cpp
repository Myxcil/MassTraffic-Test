// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficParkingSpotActor.h"

#include "MassTrafficParkingSpotComponent.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
AMassTrafficParkingSpotActor::AMassTrafficParkingSpotActor(const FObjectInitializer& ObjectInitializer)
{
	ParkingSpotComponent = CreateDefaultSubobject<UMassTrafficParkingSpotComponent>("ParkingSpotComponent");
}
