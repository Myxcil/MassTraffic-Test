// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficTrackNearVehiclesVisualizer.h"

#include "MassTrafficTrackNearVehicles.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficTrackNearVehiclesVisualizer::FMassTrafficTrackNearVehiclesVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficTrackNearVehiclesVisualizer::~FMassTrafficTrackNearVehiclesVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void FMassTrafficTrackNearVehiclesVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UMassTrafficTrackNearVehicles* NearVehiclesCmp = Cast<UMassTrafficTrackNearVehicles>(Component);
	if (!NearVehiclesCmp)
		return;

	const FNearestVehicleInfo& Info = NearVehiclesCmp->GetNearestVehicleInfo();
	if (!Info.Handle.IsValid())
		return;

	const FTransform& OwnerTransform = NearVehiclesCmp->GetOwner()->GetTransform();
	PDI->DrawLine( OwnerTransform.GetLocation(), Info.Position, FLinearColor::Red, SDPG_Foreground, 4.0f);
}
