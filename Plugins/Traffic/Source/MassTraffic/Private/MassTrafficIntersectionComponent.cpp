// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficIntersectionComponent.h"

#include "MassTrafficSubsystem.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
namespace
{
	TMap<FZoneGraphLaneHandle, TObjectPtr<UMassTrafficIntersectionComponent>> LaneHandleToIntersectionMap;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UMassTrafficIntersectionComponent::UMassTrafficIntersectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::OnRegister()
{
	Super::OnRegister();
	RefreshLanes();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::RefreshLanes()
{
	LaneHandles.Reset();
	BlockingLaneIndices.Reset();
	IntersectionSides.Reset();

	MassTrafficSubsystem = UWorld::GetSubsystem<UMassTrafficSubsystem>(GetWorld());
	if (MassTrafficSubsystem)
	{
		ZoneGraphSubsystem = UWorld::GetSubsystem<UZoneGraphSubsystem>(GetWorld());
		if (ZoneGraphSubsystem)
		{
			const TIndirectArray<FMassTrafficZoneGraphData>& ZoneGraphDataArray = MassTrafficSubsystem->GetTrafficZoneGraphData();
			if (ZoneGraphDataArray.Num() == 0)
			{
				UE_LOG(LogMassTraffic, Warning, TEXT("No Zonegraph in scene, deactivating Intersection for %s"), *GetOwner()->GetName())
				return;
			}

			const UMassTrafficSettings* MassTrafficSettings = GetDefault<UMassTrafficSettings>();
			check(MassTrafficSettings);

			// get all intersection lanes inside the given area
			const FBox QueryBounds = FBox::BuildAABB(GetOwner()->GetActorLocation(), FVector(IntersectionSize));
			if (ZoneGraphSubsystem->FindOverlappingLanes(QueryBounds, MassTrafficSettings->IntersectionLaneFilter, LaneHandles))
			{
				TMap<FVector, TArray<int32>> LaneIndexMap;
				TArray<TTuple<FVector,FVector>> LaneStartAndEnds;

				FVector IntersectionCenter(0,0,0);
				
				// go through all found lanes and store their index
				// based on their start positions
				const int32 NumLanes = LaneHandles.Num();
				for(int32 LaneIndex=0; LaneIndex < NumLanes; ++LaneIndex)
				{
					const FZoneGraphLaneHandle LaneHandle = LaneHandles[LaneIndex];
					
					const AZoneGraphData* ZoneGraphData = ZoneGraphSubsystem->GetZoneGraphData(LaneHandle.DataHandle);
					check(ZoneGraphData);
					const FZoneGraphStorage& ZoneStorage = ZoneGraphData->GetStorage();
					const FZoneLaneData& Lane = ZoneStorage.Lanes[LaneHandle.Index];

					const FVector LaneStartPosition = ZoneStorage.LanePoints[Lane.PointsBegin];
					TArray<int32>& LaneIndices = LaneIndexMap.FindOrAdd(LaneStartPosition);
					LaneIndices.Add(LaneIndex);

					const FVector LaneEndPosition = ZoneStorage.LanePoints[Lane.PointsEnd-1];
					LaneStartAndEnds.Add({LaneStartPosition,LaneEndPosition});
					
					IntersectionCenter += LaneStartPosition;
				}
				IntersectionCenter /= NumLanes;

				// generate road lanes, which means
				// all lanes starting from a specific point
				// will be stored in the same array
				int32 SideIndex = 0;
				RoadLanes.SetNum(LaneIndexMap.Num());
				for(const TTuple<FVector, TArray<int32>>& Iter : LaneIndexMap)
				{
					FRoadLanes& RoadLane = RoadLanes[SideIndex];
					RoadLane.Position = Iter.Key;
					RoadLane.LaneIndices = Iter.Value;
					++SideIndex;
				}
				
				// create intersection sides based on previously created map
				// store indices into LaneHandles array for each side
				SideIndex = 0;
				IntersectionSides.SetNum(LaneIndexMap.Num());
				for(const TTuple<FVector, TArray<int32>>& Iter : LaneIndexMap)
				{
					FIntersectionSide& IntersectionSide = IntersectionSides[SideIndex];
					IntersectionSide.bHasPriority = PriorityRoadSides.Contains(SideIndex);
					IntersectionSide.LaneIndices = Iter.Value;
					IntersectionSide.DirectionIntoIntersection = Iter.Key;
					++SideIndex;
				}

				SortSides();

				// Determine which lanes could be blocked by traffic on a specific lane
				BlockingLaneIndices.SetNum(NumLanes);
				for(int32 LaneIndex0=0; LaneIndex0 < NumLanes-1; ++LaneIndex0)
				{
					const TTuple<FVector,FVector>& Lane0 = LaneStartAndEnds[LaneIndex0];
					for(int32 LaneIndex1=LaneIndex0+1; LaneIndex1 < NumLanes; ++LaneIndex1)
					{
						bool bSameSide = false;
						for(int32 I=0; I < IntersectionSides.Num(); ++I)
						{
							// lanes start from the same side? skip
							if (IntersectionSides[I].LaneIndices.Contains(LaneIndex0) &&
								IntersectionSides[I].LaneIndices.Contains(LaneIndex1))
							{
								bSameSide = true;
								break;
							}
						}
						if (bSameSide)
							continue;;
						
						const TTuple<FVector,FVector>& Lane1 = LaneStartAndEnds[LaneIndex1];

						// If there is an intersection between these two segments,
						// traffic on this lanes will block each other
						// so store the lane indices in the appropriate array for each lane
						// A blocks B => B blocks A
						FVector Tmp;
						if (FMath::SegmentIntersection2D(Lane0.Key,Lane0.Value, Lane1.Key, Lane1.Value, Tmp))
						{
							BlockingLaneIndices[LaneIndex0].AddUnique(LaneIndex1);
							BlockingLaneIndices[LaneIndex1].AddUnique(LaneIndex0);
						}
					}
				}
			}
		}
	}

	// initialize sides here
	switch (IntersectionType)
	{
	case EIntersectionType::PriorityRoad:
		for(int32 I=0; I < IntersectionSides.Num(); ++I)
		{
			const bool bOpen = IntersectionSides[I].bHasPriority;
			IntersectionSides[I].bIsOpen = bOpen;
		}
		break;
		
	case EIntersectionType::PriorityRight:
		for(int32 I=0; I < IntersectionSides.Num(); ++I)
		{
			IntersectionSides[I].bHasPriority = false;
			IntersectionSides[I].bIsOpen = false;
		}
		break;

	case EIntersectionType::TrafficLights:
		for(int32 I=0; I < IntersectionSides.Num(); ++I)
		{
			IntersectionSides[I].bHasPriority = false;
			IntersectionSides[I].bIsOpen = false;
		}
		CurrentTrafficPhase = 0;
		PhaseTimeRemaining = TrafficLightSetups.Num() > 0 ? TrafficLightSetups[0].Duration : 0;
		break;
	}

	ApplyLaneStatus();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::SetEmergencyLane(const FZoneGraphLaneHandle& LaneHandle, const bool bIsEmergency)
{
	if (bIsEmergencyLaneSet == bIsEmergency)
		return;
	
	if (bIsEmergency)
	{
		/*
		for(int32 I=0; I < LaneHandles.Num(); ++I)
		{
			if (FZoneGraphTrafficLaneData* TrafficLaneData =  MassTrafficSubsystem->GetMutableTrafficLaneData(LaneHandles[I]))
			{
				TrafficLaneData->bIsOpen = LaneHandles[I] == LaneHandle;
			}
		}
		*/

		for(int32 I=0; I < IntersectionSides.Num(); ++I)
		{
			if (DoesSideContainLane(IntersectionSides[I], LaneHandle))
			{
				IntersectionSides[I].bIsOpen = true;
			}
			else
			{
				IntersectionSides[I].bIsOpen = false;
			}
		}
		ApplyLaneStatus();
	}

	bIsEmergencyLaneSet = bIsEmergency;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsEmergencyLaneSet)
		return;
	
	// first we open all lanes
	// and apply the current state each frame
	// TODO: maybe some caching here? Don't know what yet. Let's finish it first.
	OpenAllLanes();

	switch (IntersectionType)
	{
	case EIntersectionType::PriorityRoad:
		HandlePriorityRoad(DeltaTime);
		break;
		
	case EIntersectionType::PriorityRight:
		HandlePriorityRight(DeltaTime);
		break;

	case EIntersectionType::TrafficLights:
		HandleTrafficLights(DeltaTime);
		break;
	}

	if (IntersectionType != EIntersectionType::TrafficLights)
	{
		ApplyLaneStatus();
	}
	else
	{
		ApplyTrafficLightStatus();
	}

	UpdateBlockingLanes();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UMassTrafficIntersectionComponent* UMassTrafficIntersectionComponent::FindIntersection(const FZoneGraphLaneHandle& LaneHandle)
{
	TObjectPtr<UMassTrafficIntersectionComponent>* PtrIntersection = LaneHandleToIntersectionMap.Find(LaneHandle);
	return PtrIntersection ? *PtrIntersection : nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::BeginPlay()
{
	Super::BeginPlay();
	for(int32 I=0; I < LaneHandles.Num(); ++I)
	{
		LaneHandleToIntersectionMap.Add( LaneHandles[I], this);
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for(int32 I=0; I < LaneHandles.Num(); ++I)
	{
		LaneHandleToIntersectionMap.Remove( LaneHandles[I]);
	}
	Super::EndPlay(EndPlayReason);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::SortSides()
{
	TArray<FMassTrafficFloatAndID> ZAngleAndSideIndexArray;
	for (int32 S = 0; S < IntersectionSides.Num(); S++)
	{
		const FVector SideDirection = IntersectionSides[S].DirectionIntoIntersection;

		static const FVector ReferenceDirection(1.0f, 0.0f, 0.0f);
		const float Dot = FVector::DotProduct(ReferenceDirection, SideDirection);
		const FVector Cross = FVector::CrossProduct(ReferenceDirection, SideDirection);	

		const float SortSign = (Cross.Z > 0.0f ? 1.0f : -1.0f);
		const float ZAngle = FMath::Acos(Dot);
		const float SignedZAngle = SortSign * ZAngle;

		ZAngleAndSideIndexArray.Add(FMassTrafficFloatAndID(SignedZAngle, S));
	}
	
	ZAngleAndSideIndexArray.Sort();

	TArray<FIntersectionSide> OldSides = IntersectionSides;
	IntersectionSides.Empty();
	for (int32 S = 0; S < ZAngleAndSideIndexArray.Num(); S++)
	{
		const int32 SOld = ZAngleAndSideIndexArray[S].ID;
		IntersectionSides.Add(OldSides[SOld]);
	}}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::OpenAllLanes()
{
	for(int32 LaneIndex=0; LaneIndex < LaneHandles.Num(); ++LaneIndex)
	{
		if (FZoneGraphTrafficLaneData* TrafficLaneData =  MassTrafficSubsystem->GetMutableTrafficLaneData(LaneHandles[LaneIndex]))
		{
			TrafficLaneData->bIsOpen = true;
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::HandlePriorityRight(const float DeltaTime)
{
	int32 NumClosed = 0;
	
	const int32 NumSides = IntersectionSides.Num();
	for(int32 Side=0; Side < NumSides; ++Side)
	{
		const int32 NextRight = (Side + NumSides - 1) % NumSides;
		if (IsVehicleApproaching(NextRight))
		{
			IntersectionSides[Side].bIsOpen = false;
			++NumClosed;
		}
		else
		{
			IntersectionSides[Side].bIsOpen = true;
		}
	}

	// just open the first one if every one is closed
	if (NumClosed == NumSides)
	{
		IntersectionSides[0].bIsOpen = true;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::HandlePriorityRoad(const float DeltaTime)
{
	// check if there is incoming traffic from the priority sides
	bool bPriorityRoadHasTraffic = false;
	for(int32 I=0; I < IntersectionSides.Num(); ++I)
	{
		if (IntersectionSides[I].bHasPriority && IsVehicleApproaching(I))
		{
			bPriorityRoadHasTraffic = true;
			break;
		}
	}

	// set all non-priority sides to open if there is no traffic on the main road
	for(int32 I=0; I < IntersectionSides.Num(); ++I)
	{
		if (!IntersectionSides[I].bHasPriority)
		{
			IntersectionSides[I].bIsOpen = !bPriorityRoadHasTraffic;
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::HandleTrafficLights(const float DeltaTime)
{
	if (TrafficLightSetups.IsEmpty())
		return;
	
	PhaseTimeRemaining -= DeltaTime;
	if (PhaseTimeRemaining <= 0)
	{
		if (++CurrentTrafficPhase >= TrafficLightSetups.Num())
		{
			CurrentTrafficPhase = 0;
		}
		PhaseTimeRemaining = TrafficLightSetups[CurrentTrafficPhase].Duration;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::UpdateBlockingLanes()
{
	// check all lanes for active traffic
	// if there is at least one vehicle,
	// close all lanes that could be blocked
	for(int32 LaneIndex=0; LaneIndex < LaneHandles.Num(); ++LaneIndex)
	{
		if (const FZoneGraphTrafficLaneData* TrafficLaneData =  MassTrafficSubsystem->GetTrafficLaneData(LaneHandles[LaneIndex]))
		{
			if (TrafficLaneData->NumVehiclesOnLane > 0)
			{
				const TArray<int32>& BlockingLanes = BlockingLaneIndices[LaneIndex];
				for(int32 J=0; J < BlockingLanes.Num(); ++J)
				{
					FZoneGraphTrafficLaneData* LaneToBlock =  MassTrafficSubsystem->GetMutableTrafficLaneData(LaneHandles[BlockingLanes[J]]);
					LaneToBlock->bIsOpen = false;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::ApplyLaneStatus()
{
	// apply current state to lanes belonging to each side
	for(int32 I=0; I < IntersectionSides.Num(); ++I)
	{
		const FIntersectionSide& Side = IntersectionSides[I]; 

		const bool bLanesOpen = Side.bIsOpen;
		
		const int32 NumLanes = Side.LaneIndices.Num();
		for(int32 J=0; J < NumLanes; ++J)
		{
			const int32 LaneIndex = Side.LaneIndices[J]; 
			if (FZoneGraphTrafficLaneData* TrafficLaneData =  MassTrafficSubsystem->GetMutableTrafficLaneData(LaneHandles[LaneIndex]))
			{
				TrafficLaneData->bIsOpen = bLanesOpen;
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficIntersectionComponent::ApplyTrafficLightStatus()
{
	for(int32 I=0; I < TrafficLightSetups.Num(); ++I)
	{
		const bool bOpen = I == CurrentTrafficPhase;

		const FTrafficLightSetup& TrafficLightSetup = TrafficLightSetups[I];
		const int32 NumLanes = TrafficLightSetup.OpenLanes.Num();
		for(int32 J=0; J < NumLanes; ++J)
		{
			const int32 LaneIndex = TrafficLightSetup.OpenLanes[J]; 
			if (FZoneGraphTrafficLaneData* TrafficLaneData =  MassTrafficSubsystem->GetMutableTrafficLaneData(LaneHandles[LaneIndex]))
			{
				TrafficLaneData->bIsOpen = bOpen;
			}
		}
		
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficIntersectionComponent::IsVehicleApproaching(const int32 SideIndex) const
{
	// check each lane on this side for
	// incoming vehicles or vehicles already on the lane 
	const FIntersectionSide& IntersectionSide = IntersectionSides[SideIndex];
	const int32 NumLanes = IntersectionSide.LaneIndices.Num();
	for(int32 I=0; I < NumLanes; ++I)
	{
		const int32 LaneIndex = IntersectionSide.LaneIndices[I];
		if (const FZoneGraphTrafficLaneData* TrafficLaneData =  MassTrafficSubsystem->GetTrafficLaneData(LaneHandles[LaneIndex]))
		{
			if (TrafficLaneData->bIsVehicleReadyToUseLane ||
				TrafficLaneData->NumVehiclesApproachingLane > 0 ||
				TrafficLaneData->NumVehiclesOnLane > 0)
			{
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficIntersectionComponent::DoesSideContainLane(const FIntersectionSide& Side, const FZoneGraphLaneHandle& LaneHandle) const
{
	for(int32 I=0; I < Side.LaneIndices.Num(); ++I)
	{
		const FZoneGraphLaneHandle& SideLaneHandle = LaneHandles[Side.LaneIndices[I]];
		if (SideLaneHandle == LaneHandle)
			return true;
	}
	return false;
}
