// Copyright 2024 Crenetic GmbH Studios All rights reserved.


#include "MassTrafficParkingSpotComponentVisualizer.h"

#include "MassTrafficParkingSpotComponent.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficParkingSpotComponentVisualizer::FMassTrafficParkingSpotComponentVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficParkingSpotComponentVisualizer::~FMassTrafficParkingSpotComponentVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void FMassTrafficParkingSpotComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UMassTrafficParkingSpotComponent* ParkingSpotComponent = Cast<UMassTrafficParkingSpotComponent>(Component);
	if (!ParkingSpotComponent)
		return;

	const AActor* Owner = ParkingSpotComponent->GetOwner();
	if (!Owner)
		return;

	const FVector2D& Size = ParkingSpotComponent->GetSize();
	const FVector Center = Owner->GetActorLocation();
	const FVector HalfFwd = 0.5 * Size.X * Owner->GetActorForwardVector();
	const FVector HalfRight = 0.5 * Size.Y * Owner->GetActorRightVector();

	const FLinearColor Color = FColor::Cyan;
	PDI->DrawLine(Center - HalfFwd - HalfRight, Center + HalfFwd - HalfRight, Color, SDPG_Foreground, 2.0f);
	PDI->DrawLine(Center + HalfFwd - HalfRight, Center + HalfFwd + HalfRight, Color, SDPG_Foreground, 2.0f);
	PDI->DrawLine(Center + HalfFwd + HalfRight, Center - HalfFwd + HalfRight, Color, SDPG_Foreground, 2.0f);
	PDI->DrawLine(Center - HalfFwd + HalfRight, Center - HalfFwd - HalfRight, Color, SDPG_Foreground, 2.0f);
}
