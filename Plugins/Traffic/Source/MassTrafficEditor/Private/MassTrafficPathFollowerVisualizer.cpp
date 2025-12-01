// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficPathFollowerVisualizer.h"

#include "MassTrafficPathFollower.h"
#include "ZoneGraphRenderingUtilities.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficPathFollowerVisualizer::FMassTrafficPathFollowerVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficPathFollowerVisualizer::~FMassTrafficPathFollowerVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void FMassTrafficPathFollowerVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UMassTrafficPathFollower* PathFinder = Cast<const UMassTrafficPathFollower>(Component);
	if (!PathFinder)
		return;

	const FTransform& OwnerTransform = PathFinder->GetOwner()->GetTransform();
	const FVector OwnerLocation = OwnerTransform.GetLocation(); 
	
	FVector LanePosition;
	if (PathFinder->GetLastLaneLocation(LanePosition))
	{
		PDI->DrawLine(OwnerLocation, LanePosition, FLinearColor::Green, SDPG_Foreground, 2.0f);
	}

	FVector TargetPosition;
	FQuat TargetOrientation;
	PathFinder->GetLastTarget(TargetPosition, TargetOrientation);

	PDI->DrawLine( OwnerLocation, TargetPosition, FLinearColor::Red, SDPG_Foreground, 2.0f);
	DrawCoordinateSystem( PDI, OwnerLocation, TargetOrientation.Rotator(), 100.0f, SDPG_Foreground, 1.0f);

	const FVector Offset(0,0,50.0); 
	PathFinder->ForEachLaneInPath([&](const FZoneGraphTrafficLaneData* Lane)
	{
		const FZoneGraphStorage* Storage = PathFinder->GetZoneGraphStorage(Lane->LaneHandle);
		UE::ZoneGraph::RenderingUtilities::DrawLane(*Storage, PDI, Lane->LaneHandle, PathFinder->GetPathDebugColor(), 2, Offset);
	});
	
	constexpr float Radius = 50.0f;
	DrawWireSphere(PDI, PathFinder->GetOrigin().Position, FLinearColor::Red, Radius, 16, SDPG_Foreground, 1.0f);
	DrawWireSphere(PDI, PathFinder->GetDestination().Position, FLinearColor::Green, Radius, 16, SDPG_Foreground, 1.0f);
}
