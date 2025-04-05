// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficPathFinderVisualizer.h"

#include "MassTrafficPathFinder.h"
#include "ZoneGraphRenderingUtilities.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficPathFinderVisualizer::FMassTrafficPathFinderVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficPathFinderVisualizer::~FMassTrafficPathFinderVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void FMassTrafficPathFinderVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UMassTrafficPathFinder* PathFinder = Cast<const UMassTrafficPathFinder>(Component);
	if (!PathFinder)
		return;

	if (FVector LanePosition; PathFinder->GetLastLaneLocation(LanePosition))
	{
		const FTransform& OwnerTransform = PathFinder->GetOwner()->GetTransform();
		PDI->DrawLine(OwnerTransform.GetLocation(), LanePosition, FLinearColor::Green, SDPG_Foreground, 2.0f);
	}

	const TConstArrayView<const FZoneGraphTrafficLaneData*> Lanes = PathFinder->GetLastCalculatedPath();
	if (Lanes.Num() == 0)
		return;

	const FVector Offset(0,0,50.0); 
	for(int32 I=0; I < Lanes.Num(); ++I)
	{
		const FZoneGraphTrafficLaneData* Lane = Lanes[I];
		const FZoneGraphStorage* Storage = PathFinder->GetZoneGraphStorage(Lane->LaneHandle);
		UE::ZoneGraph::RenderingUtilities::DrawLane(*Storage, PDI, Lane->LaneHandle, PathFinder->GetPathDebugColor(), 2, Offset);
	}

	constexpr float Radius = 50.0f;
	DrawWireSphere(PDI, PathFinder->GetOrigin().Position, FLinearColor::Red, Radius, 16, SDPG_Foreground, 1.0f);
	DrawWireSphere(PDI, PathFinder->GetDestination().Position, FLinearColor::Green, Radius, 16, SDPG_Foreground, 1.0f);
}
