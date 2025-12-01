// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassTrafficPathTypes.h"

class UZoneGraphSubsystem;
class UMassTrafficSubsystem;
struct FZoneGraphTrafficLaneData;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
class MASSTRAFFIC_API FMassTrafficPathFinder
{
public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	FMassTrafficPathFinder();
	~FMassTrafficPathFinder();

	bool Init(UMassTrafficSubsystem* InMassTrafficSubsystem, UZoneGraphSubsystem* InZoneGraphSubsystem, FZoneGraphTagFilter ZoneGraphTagFilter, float LaneSearchRadius);
	bool SearchPath(const FVector& Start, const FVector& End, FTrafficPath& TrafficPath);
	
	bool FindNearestLane(const FVector& Location, const float SearchSize, FZoneGraphLaneLocation& LaneLocation) const;
	const FZoneGraphTrafficLaneData* GetLaneData(const FZoneGraphLaneHandle& LaneHandle) const;
	bool GetRandomLocation(FVector& Position) const;

	static float CalculatePathLength(const FTrafficPath& TrafficPath);

private:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	struct FLaneNode
	{
		uint32 SearchIndex;
		bool bIsClosed;
		const FZoneGraphTrafficLaneData* Parent = nullptr;
		float CostFromStart;
		float EstimateCostToGoal;
		float TotalCost;
	};
	
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	// Get (or updates) node for given lane ptr
	FLaneNode& GetNode(const FZoneGraphTrafficLaneData* Lane);
	// Returns the lane with the lowest TotalCost in current OpenList and removes it 
	const FZoneGraphTrafficLaneData* PopCheapest();
	// Check lane connections and update OpenList according to A*
	void EvaluateLane(const FZoneGraphTrafficLaneData* Lane, const FZoneGraphTrafficLaneData* To);
	
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UMassTrafficSubsystem* MassTrafficSubsystem = nullptr;
	UZoneGraphSubsystem* ZoneGraphSubsystem = nullptr;
	FZoneGraphTagFilter ZoneGraphTagFilter;
	float LaneSearchRadius = 0;

	TArray<const FZoneGraphTrafficLaneData*> Lanes;
	TMap<const FZoneGraphTrafficLaneData*, FLaneNode> LaneNodes;
	TArray<const FZoneGraphTrafficLaneData*> OpenList;
	uint32 CurrentSearchIndex = 0;
};
