#pragma once

#include "MassTrafficTypes.h"
#include "ZoneGraphTypes.h"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct FTrafficPath
{
	FZoneGraphLaneLocation Origin;
	FZoneGraphLaneLocation Destination;
	TArray<const FZoneGraphTrafficLaneData*> Path;
	float TotalLength = 0;

	void Reset() { Origin.Reset(); Destination.Reset(); Path.Reset(); }
	bool IsValid() const { return Origin.IsValid() && Destination.IsValid() && Path.Num() > 2; }
};
	
