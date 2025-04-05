// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficIntersectionComponentVisualizer.h"
#include "MassTrafficIntersectionComponent.h"
#include "MassTrafficSettings.h"
#include "MassTrafficSubsystem.h"
#include "ZoneGraphRenderingUtilities.h"
#include "ZoneGraphSubsystem.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficIntersectionComponentVisualizer::FMassTrafficIntersectionComponentVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficIntersectionComponentVisualizer::~FMassTrafficIntersectionComponentVisualizer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void FMassTrafficIntersectionComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UMassTrafficIntersectionComponent* IntersectionComponent = Cast<UMassTrafficIntersectionComponent>(Component);
	if (!IntersectionComponent)
		return;

	const UMassTrafficSubsystem* MassTrafficSubsystem = IntersectionComponent->GetMassTrafficSubsystem();
	if (!MassTrafficSubsystem)
		return;

	const UZoneGraphSubsystem* ZoneGraphSubsystem = IntersectionComponent->GetZoneGraphSubsystem();
	if (!ZoneGraphSubsystem)
		return;

	const TArray<FZoneGraphLaneHandle>& LaneHandles = IntersectionComponent->GetLaneHandles();
	if (LaneHandles.IsEmpty())
		return;

	DrawCircle(PDI, IntersectionComponent->GetOwner()->GetActorLocation(), FVector::XAxisVector, FVector::YAxisVector, FLinearColor::White, IntersectionComponent->GetIntersectionSize(), 32, SDPG_Foreground, 1.0f);

	if (IntersectionComponent->GetIntersectionType() != EIntersectionType::TrafficLights)
	{
		const int32 NumSides = IntersectionComponent->GetNumSides();
		for(int32 Side=0; Side < NumSides; ++Side)
		{
			const bool bIsPrioritySide = IntersectionComponent->IsPrioritySide(Side);

			const TArray<int32>& LaneIndices = IntersectionComponent->GetSideLaneIndices(Side);
			for(int32 I=0; I < LaneIndices.Num(); ++I)
			{
				const FZoneGraphLaneHandle LaneHandle = LaneHandles[LaneIndices[I]];
				if (!LaneHandle.IsValid())
					continue;
		
				const AZoneGraphData* ZoneGraphData = ZoneGraphSubsystem->GetZoneGraphData(LaneHandle.DataHandle);
				if (!ZoneGraphData)
					continue;

				const FZoneGraphTrafficLaneData* TrafficLaneData = MassTrafficSubsystem->GetTrafficLaneData(LaneHandle);
				if (!TrafficLaneData)
					continue;

				const FColor Color = TrafficLaneData->bIsOpen ? (bIsPrioritySide ? FColor::Yellow : FColor::Green) : FColor::Red;  

				const FVector Offset(0,0,5.0f);
				UE::ZoneGraph::RenderingUtilities::DrawLane( ZoneGraphData->GetStorage(), PDI, LaneHandle, Color, 2.0f, Offset);
			}
		}
	}
	else
	{
		const TArray<FTrafficLightSetup>& TrafficLightSetups = IntersectionComponent->GetTrafficLightSetups();
		const int32 NumTrafficPhases = TrafficLightSetups.Num();
		for(int32 I=0; I < NumTrafficPhases; ++I)
		{
			if (!TrafficLightSetups[I].bShow)
				continue;;
			
			const TArray<int32>& LaneIndices = TrafficLightSetups[I].OpenLanes;
			for(int32 J=0; J < LaneIndices.Num(); ++J)
			{
				const FZoneGraphLaneHandle LaneHandle = LaneHandles[LaneIndices[J]];
				if (!LaneHandle.IsValid())
					continue;
		
				const AZoneGraphData* ZoneGraphData = ZoneGraphSubsystem->GetZoneGraphData(LaneHandle.DataHandle);
				if (!ZoneGraphData)
					continue;

				const FZoneGraphTrafficLaneData* TrafficLaneData = MassTrafficSubsystem->GetTrafficLaneData(LaneHandle);
				if (!TrafficLaneData)
					continue;

				const FVector Offset(0,0,5.0f);
				UE::ZoneGraph::RenderingUtilities::DrawLane( ZoneGraphData->GetStorage(), PDI, LaneHandle, FColor::Green, 2.0f, Offset);
			}
		}
	}
}
