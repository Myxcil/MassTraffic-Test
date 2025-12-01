// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficPathFinderVisualizer.h"

#include "MassTrafficPathFinder.h"
#include "MassTrafficPathFollower.h"
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
	const UMassTrafficPathFollower* PathFollower = Cast<const UMassTrafficPathFollower>(Component);
	if (!PathFollower)
		return;

	const FTransform& OwnerTransform = PathFollower->GetOwner()->GetTransform();
	const FVector OwnerLocation = OwnerTransform.GetLocation(); 
	
	FVector LanePosition;
	if (PathFollower->GetLastLaneLocation(LanePosition))
	{
		PDI->DrawLine(OwnerLocation, LanePosition, FLinearColor::Green, SDPG_Foreground, 2.0f);
	}

	FVector TargetPosition;
	FQuat TargetOrientation;
	PathFollower->GetLastTarget(TargetPosition, TargetOrientation);

	PDI->DrawLine( OwnerLocation, TargetPosition, FLinearColor::Red, SDPG_Foreground, 2.0f);
	DrawCoordinateSystem( PDI, OwnerLocation, TargetOrientation.Rotator(), 100.0f, SDPG_Foreground, 1.0f);

	const FVector Offset(0,0,50.0); 
	PathFollower->ForEachLaneInPath([&](const FZoneGraphTrafficLaneData* Lane)
	{
		const FZoneGraphStorage* Storage = PathFollower->GetZoneGraphStorage(Lane->LaneHandle);
		UE::ZoneGraph::RenderingUtilities::DrawLane(*Storage, PDI, Lane->LaneHandle, PathFollower->GetPathDebugColor(), 2, Offset);
	});
	
	constexpr float Radius = 50.0f;
	DrawWireSphere(PDI, PathFollower->GetOrigin().Position, FLinearColor::Red, Radius, 16, SDPG_Foreground, 1.0f);
	DrawWireSphere(PDI, PathFollower->GetDestination().Position, FLinearColor::Green, Radius, 16, SDPG_Foreground, 1.0f);
}
