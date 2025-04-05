// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficProxyActor.h"

#include "EngineUtils.h"


// Sets default values
AMassTrafficProxyActor::AMassTrafficProxyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	CreateDefaultSubobject<USceneComponent>("Root");
}
