// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficPathFinder.h"

#include "MassTrafficFragments.h"
#include "MassTrafficInterpolation.h"
#include "MassTrafficSubsystem.h"
#include "MassTrafficUtils.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficPathFinder::FMassTrafficPathFinder()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficPathFinder::~FMassTrafficPathFinder()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool FMassTrafficPathFinder::Init(UMassTrafficSubsystem* InMassTrafficSubsystem, UZoneGraphSubsystem* InZoneGraphSubsystem, FZoneGraphTagFilter InZoneGraphTagFilter,float InLaneSearchRadius)
{
	MassTrafficSubsystem = InMassTrafficSubsystem;
	ZoneGraphSubsystem = InZoneGraphSubsystem;
	ZoneGraphTagFilter = InZoneGraphTagFilter;
	LaneSearchRadius = InLaneSearchRadius;
	
	const TIndirectArray<FMassTrafficZoneGraphData>& ZoneGraphDataArray = MassTrafficSubsystem->GetTrafficZoneGraphData();
	if (ZoneGraphDataArray.IsEmpty())
		return false;
	
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
	
	Lanes.SetNum(LaneNodes.Num());
	LaneNodes.GetKeys(Lanes);
	
	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool FMassTrafficPathFinder::SearchPath(const FVector& Start, const FVector& End, FTrafficPath& TrafficPath)
{
	if (!FindNearestLane(Start, LaneSearchRadius, TrafficPath.Origin))
		return false;
	
	if (!FindNearestLane(End, LaneSearchRadius, TrafficPath.Destination))
		return false;

	const FZoneGraphTrafficLaneData* From = GetLaneData(TrafficPath.Origin.LaneHandle);
	if (!From)
		return false;
	
	const FZoneGraphTrafficLaneData* To = GetLaneData(TrafficPath.Destination.LaneHandle);
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
			TrafficPath.Path.Reset();
			while (Lane != nullptr)
			{
				TrafficPath.Path.Add(Lane);
				Lane = LaneNodes[Lane].Parent;
			}
			Algo::Reverse(TrafficPath.Path);
			TrafficPath.TotalLength = CalculatePathLength(TrafficPath);
			return true;
		}
		LaneNodes[Lane].bIsClosed = true;
		EvaluateLane(Lane, To);
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool FMassTrafficPathFinder::FindNearestLane(const FVector& Location, const float SearchSize, FZoneGraphLaneLocation& LaneLocation) const
{
	const FBox SearchBox = FBox::BuildAABB(Location, FVector(SearchSize));
	float Tmp;
	return ZoneGraphSubsystem->FindNearestLane(SearchBox, ZoneGraphTagFilter, LaneLocation, Tmp);
}


//------------------------------------------------------------------------------------------------------------------------------------------------------------
FMassTrafficPathFinder::FLaneNode& FMassTrafficPathFinder::GetNode(const FZoneGraphTrafficLaneData* Lane)
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
const FZoneGraphTrafficLaneData* FMassTrafficPathFinder::PopCheapest()
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
void FMassTrafficPathFinder::EvaluateLane(const FZoneGraphTrafficLaneData* Lane, const FZoneGraphTrafficLaneData* To)
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

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphTrafficLaneData* FMassTrafficPathFinder::GetLaneData(const FZoneGraphLaneHandle& LaneHandle) const
{
	return MassTrafficSubsystem->GetTrafficLaneData(LaneHandle);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool FMassTrafficPathFinder::GetRandomLocation(FVector& Position) const
{
	const int32 RandomLaneIndex = FMath::RandRange(0, Lanes.Num()-1);
	if (!Lanes.IsValidIndex(RandomLaneIndex))
		return false;
	
	const FZoneGraphTrafficLaneData* LaneData = Lanes[RandomLaneIndex];
	if (!LaneData)
		return false;
	
	const FZoneGraphStorage* ZoneGraphStorage = ZoneGraphSubsystem->GetZoneGraphStorage(LaneData->LaneHandle.DataHandle);
	if (!ZoneGraphStorage)
		return false;
	
	const float Length = UE::MassTraffic::GetLaneBeginToEndDistance(RandomLaneIndex, *ZoneGraphStorage);

	FMassTrafficPositionOnlyLaneSegment LaneSegment;
	UE::MassTraffic::InterpolatePositionAlongLane(*ZoneGraphStorage, RandomLaneIndex, 0.5f * Length, ETrafficVehicleMovementInterpolationMethod::CubicBezier, LaneSegment, Position);
	
	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
float FMassTrafficPathFinder::CalculatePathLength(const FTrafficPath& TrafficPath)
{
	float Length = TrafficPath.Path[0]->Length - TrafficPath.Origin.DistanceAlongLane;
	for(int32 I=1; I < TrafficPath.Path.Num()-1; ++I)
	{
		Length += TrafficPath.Path[I]->Length;
	}
	Length += TrafficPath.Destination.DistanceAlongLane;
	return Length;
}
