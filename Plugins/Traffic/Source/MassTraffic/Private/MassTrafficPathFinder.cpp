// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficPathFinder.h"

#include "MassTrafficInterpolation.h"
#include "MassTrafficSubsystem.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficPathFinder::BeginPlay()
{
	Super::BeginPlay();

	MassTrafficSubsystem = GetWorld()->GetSubsystem<UMassTrafficSubsystem>();
	check(MassTrafficSubsystem);

	MassTrafficSettings = GetDefault<UMassTrafficSettings>();

	ZoneGraphSubsystem = GetWorld()->GetSubsystem<UZoneGraphSubsystem>();

	const TIndirectArray<FMassTrafficZoneGraphData>& ZoneGraphDataArray = MassTrafficSubsystem->GetTrafficZoneGraphData();
	if (ZoneGraphDataArray.Num() == 0)
	{
		UE_LOG(LogMassTraffic, Warning, TEXT("No Zonegraph in scene, deactivating PathFinder for %s"), *GetOwner()->GetName())
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}

	for(int32 I=0; I < ZoneGraphDataArray.Num(); ++I)
	{
		const TConstArrayView<FZoneGraphTrafficLaneData>& LaneDataArray = ZoneGraphDataArray[I].TrafficLaneDataArray; 
		const int32 NumTrafficLaneData = LaneDataArray.Num();
	
		for(int32 J=0; J < NumTrafficLaneData; ++J)
		{
			const FZoneGraphTrafficLaneData& LaneData = LaneDataArray[J];
			LaneNodes.Add(&LaneData, FLaneNode());
		}
	}

	FindNearestLane(GetOwner()->GetActorLocation(), 500.0f, CurrLocation);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficPathFinder::FindNearestLane(const FVector& Location, const float SearchSize, FZoneGraphLaneLocation& LaneLocation) const
{
	const FBox SearchBox = FBox::BuildAABB(Location, FVector(SearchSize));
	float Tmp;
	return ZoneGraphSubsystem->FindNearestLane(SearchBox, ZoneGraphTagFilter, LaneLocation, Tmp);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficPathFinder::SearchPath(const FVector& End)
{
	if (!FindNearestLane(GetOwner()->GetActorLocation(),500.0f, Origin))
		return false;
	
	if (!FindNearestLane(End, 500.0f, Destination))
		return false;

	const FZoneGraphTrafficLaneData* From = GetLaneData(Origin.LaneHandle);
	if (!From)
		return false;
	
	const FZoneGraphTrafficLaneData* To = GetLaneData(Destination.LaneHandle);
	if (!To)
		return false;

	++CurrentSearchIndex;
	OpenList.Reset();
	
	FLaneNode& FromNode = GetNode(From);
	FromNode.CostFromStart = 0.0f;
	FromNode.EstimateCostToGoal = FVector::Dist(From->CenterLocation, To->CenterLocation);
	FromNode.TotalCost = FromNode.CostFromStart + FromNode.EstimateCostToGoal;

	OpenList.Push(From);

	while (!OpenList.IsEmpty())
	{
		const FZoneGraphTrafficLaneData* Lane = PopCheapest();
		if (Lane == To)
		{
			LanePath.Reset();
			while (Lane != nullptr)
			{
				LanePath.Add(Lane);
				Lane = LaneNodes[Lane].Parent;
			}
			Algo::Reverse(LanePath);
			return true;
		}
		LaneNodes[Lane].bIsClosed = true;
		EvaluateLane(Lane, To);
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficPathFinder::InitPathFollowing()
{
	LanePathIndex = 0;
	CurrLocation = Origin;

	if (OnLaneChanged.IsBound())
	{
		OnLaneChanged.Execute(FZoneGraphLaneHandle(), Origin.LaneHandle);
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficPathFinder::UpdatePathFollowing(const float LookAheadDistance, FVector& TargetPosition, FQuat& TargetOrientation)
{
	const int32 PrevLanePathIndex = LanePathIndex;
	const FTransform& Transform = GetOwner()->GetTransform();
	
	const FVector Location = Transform.GetLocation();
	FindNearestLane(Location, 500.0f, CurrLocation);

	// Done?
	if (CurrLocation.LaneHandle == Destination.LaneHandle && CurrLocation.DistanceAlongLane >= Destination.DistanceAlongLane)
	{
		if (OnLaneChanged.IsBound())
		{
			OnLaneChanged.Execute(CurrLocation.LaneHandle, FZoneGraphLaneHandle());
		}
		return false;
	}

	const FZoneGraphTrafficLaneData* CurrLane = LanePath[LanePathIndex];
	if (CurrLocation.LaneHandle != CurrLane->LaneHandle)
	{
		for(int32 I=LanePathIndex+1; I < LanePath.Num(); ++I)
		{
			if (LanePath[I]->LaneHandle == CurrLocation.LaneHandle)
			{
				LanePathIndex = I;
				CurrLane = LanePath[LanePathIndex];
				break;
			}
		}
	}

	int32 NextLaneIndex = INDEX_NONE;
	if (LanePathIndex < LanePath.Num()-1)
	{
		NextLaneIndex = LanePath[LanePathIndex+1]->LaneHandle.Index;
	}

	if (PrevLanePathIndex != LanePathIndex)
	{
		if (OnLaneChanged.IsBound())
		{
			OnLaneChanged.Execute(LanePath[PrevLanePathIndex]->LaneHandle, LanePath[LanePathIndex]->LaneHandle);
		}
	}

	FMassTrafficLaneSegment LaneSegment;
	UE::MassTraffic::InterpolatePositionAndOrientationAlongContinuousLanes(
		*GetZoneGraphStorage(CurrLane->LaneHandle),
		CurrLane->LaneHandle.Index,
		CurrLane->Length,
		NextLaneIndex,
		CurrLocation.DistanceAlongLane + LookAheadDistance,
		ETrafficVehicleMovementInterpolationMethod::CubicBezier,
		LaneSegment,
		TargetPosition,
		TargetOrientation);

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphTrafficLaneData* UMassTrafficPathFinder::GetCurrentLane() const
{
	if (!CurrLocation.IsValid())
		return nullptr;

	return GetLaneData(CurrLocation.LaneHandle);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphTrafficLaneData* UMassTrafficPathFinder::GetNextLane() const
{
	return LanePathIndex < LanePath.Num()-1 ? LanePath[LanePathIndex+1] : nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
float UMassTrafficPathFinder::UpdateLaneLength(const FZoneGraphTrafficLaneData* CurrLane) const
{
	if (CurrLane->LaneHandle == Destination.LaneHandle)
	{
		return Destination.DistanceAlongLane + DestinationLaneOffset;
	}
	return CurrLane->Length;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
float UMassTrafficPathFinder::GetDistanceToNextLane() const
{
	if (CurrLocation.IsValid() && LanePath.IsValidIndex(LanePathIndex))
	{
		const FZoneGraphTrafficLaneData* CurrLane = LanePath[LanePathIndex];
		return CurrLane->Length - CurrLocation.DistanceAlongLane; 		
	}
	return std::numeric_limits<float>().max();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficPathFinder::SetEmergencyLane(const FZoneGraphLaneHandle& LaneHandle, const bool bIsEmergencyLane)
{
	if (MassTrafficSubsystem)
	{
		if (FZoneGraphTrafficLaneData* TrafficLaneData = MassTrafficSubsystem->GetMutableTrafficLaneData(LaneHandle))
		{
			TrafficLaneData->bIsEmergencyLane = bIsEmergencyLane;
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphTrafficLaneData* UMassTrafficPathFinder::GetLaneData(const FZoneGraphLaneHandle& LaneHandle) const
{
	return MassTrafficSubsystem->GetTrafficLaneData(LaneHandle);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphStorage* UMassTrafficPathFinder::GetZoneGraphStorage(const FZoneGraphLaneHandle& LaneHandle) const
{
	return ZoneGraphSubsystem->GetZoneGraphStorage(LaneHandle.DataHandle);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UMassTrafficPathFinder::FLaneNode& UMassTrafficPathFinder::GetNode(const FZoneGraphTrafficLaneData* Lane)
{
	FLaneNode& Node = LaneNodes[Lane];
	if (Node.SearchIndex != CurrentSearchIndex)
	{
		Node.SearchIndex = CurrentSearchIndex;
		Node.bIsClosed = false;
		Node.Parent = nullptr;
		Node.CostFromStart = 0.0f;
		Node.EstimateCostToGoal = 0.0f;
		Node.TotalCost = 0.0f;
	}
	return Node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphTrafficLaneData* UMassTrafficPathFinder::PopCheapest()
{
	float MinCost = TNumericLimits<float>::Max();
	const int32 NumOpenLanes = OpenList.Num();
	int32 MinCostIndex = -1;
	for(int32 I=0; I < NumOpenLanes; ++I)
	{
		const FZoneGraphTrafficLaneData* Lane = OpenList[I];
		const FLaneNode& Node = LaneNodes[Lane];
		if (Node.TotalCost < MinCost)
		{
			MinCost = Node.TotalCost;
			MinCostIndex = I;
		}
	}
	ensure(MinCostIndex >= 0 && MinCostIndex < NumOpenLanes);
	const FZoneGraphTrafficLaneData* Lane = OpenList[MinCostIndex];
	OpenList.RemoveAt(MinCostIndex);

	return Lane;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficPathFinder::EvaluateLane(const FZoneGraphTrafficLaneData* Lane, const FZoneGraphTrafficLaneData* To)
{
	const FLaneNode& LaneNode = LaneNodes[Lane];
	for(int32 I=0; I < Lane->NextLanes.Num(); ++I)
	{
		const FZoneGraphTrafficLaneData* NextLane = Lane->NextLanes[I];
		FLaneNode& NextNode = GetNode(NextLane);
		if (NextNode.bIsClosed)
			continue;

		const float CostFromStart = LaneNode.CostFromStart + NextLane->Length;
		
		if (!OpenList.Contains(NextLane))
		{
			NextNode.Parent = Lane;
			NextNode.CostFromStart = CostFromStart;
			NextNode.EstimateCostToGoal = FVector::Dist(NextLane->CenterLocation, To->CenterLocation);
			NextNode.TotalCost = NextNode.CostFromStart + NextNode.EstimateCostToGoal;

			OpenList.Add(NextLane);
		}
		else if (CostFromStart < NextNode.CostFromStart)
		{
			NextNode.Parent = Lane;
			NextNode.CostFromStart = CostFromStart;
			NextNode.TotalCost = NextNode.CostFromStart + NextNode.EstimateCostToGoal;

			OpenList.Add(NextLane);
		}
	}
}
