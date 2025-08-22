// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassTrafficTypes.h"
#include "MassTrafficPathFinder.generated.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------
class UZoneGraphSubsystem;
class UMassTrafficSettings;
class UMassTrafficSubsystem;
struct FZoneGraphTrafficLaneData;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//  Component for path finding and tracking from a MassTraffic zone graph
//
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASSTRAFFIC_API UMassTrafficPathFinder : public UActorComponent
{
	GENERATED_BODY()

public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	virtual void BeginPlay() override;

	bool SearchShortestPath(const TArray<FVector>& Starts, const TArray<FVector>& Ends);
	bool SearchPath(const FVector& Start, const FVector& End);
	
	void InitPathFollowing();
	bool UpdatePathFollowing(const float LookAheadDistance, FVector& TargetPosition, FQuat& TargetOrientation);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	const FZoneGraphLaneLocation& GetOrigin() const { return CurrentPath.Origin; }
	const FZoneGraphLaneLocation& GetDestination() const { return CurrentPath.Destination; }
	
	const FZoneGraphStorage* GetZoneGraphStorage(const FZoneGraphLaneHandle& LaneHandle) const;

	const FZoneGraphLaneLocation& GetCurrentLocation() const { return CurrLocation; }
	const FZoneGraphTrafficLaneData* GetCurrentLane() const;
	const FZoneGraphTrafficLaneData* GetNextLane() const;

	bool GetLastLaneLocation(FVector& Location) const { Location =  CurrLocation.Position; return CurrLocation.IsValid(); }
	float CalculateActualLaneLength(const FZoneGraphTrafficLaneData* CurrLane) const;
	float GetDistanceToNextLane() const;

	void SetEmergencyLane(const FZoneGraphLaneHandle& LaneHandle, const bool bIsEmergencyLane);

	DECLARE_DELEGATE_TwoParams(FLaneChanged, const FZoneGraphLaneHandle&, const FZoneGraphLaneHandle&);
	FLaneChanged OnLaneChanged;
	
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
#if WITH_EDITOR
	template<typename Func>
	void ForEachLaneInPath(const Func& Functor) const { for(int32 I=0; I < CurrentPath.Path.Num(); ++I) Functor(CurrentPath.Path[I]); }
	const FColor& GetPathDebugColor() const { return PathDebugColor; }
	void GetLastTarget(FVector& Position, FQuat& Rotation) const { Position = LastTargetPosition; Rotation = LastTargetOrientation; }	
#endif

protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, meta=(Tooltip="Which zone graph tags should be used for path finding"))
	FZoneGraphTagFilter ZoneGraphTagFilter;
	UPROPERTY(EditDefaultsOnly, meta=(Tooltip="Needed to offset the distance along destination lane or car will stop too soon"))
	float LaneSearchRadius = 500.0f;
	UPROPERTY(EditDefaultsOnly, meta=(Tooltip="Needed to offset the distance along destination lane or car will stop too soon"))
	float DestinationLaneOffset = 400.0f;
#if WITH_EDITORONLY_DATA	
	UPROPERTY(EditAnywhere)
	FColor PathDebugColor = FColor::Yellow;
#endif

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	struct FTrafficPath
	{
		FZoneGraphLaneLocation Origin;
		FZoneGraphLaneLocation Destination;
		TArray<const FZoneGraphTrafficLaneData*> Path;

		void Reset() { Origin.Reset(); Destination.Reset(); Path.Reset(); }
		bool IsValid() const { return Origin.IsValid() && Destination.IsValid() && Path.Num() > 2; }
	};
	
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

	bool SearchPath(const FVector& Start, const FVector& End, FTrafficPath& TrafficPath);

	// Returns nearest lane to given location (world-space), search area affected by LaneSearchExtents
	bool FindNearestLane(const FVector& Location, const float SearchSize, FZoneGraphLaneLocation& LaneLocation) const;
	// Get corresponding lane data for handle
	const FZoneGraphTrafficLaneData* GetLaneData(const FZoneGraphLaneHandle& LaneHandle) const;
	// Get (or updates) node for given lane ptr
	FLaneNode& GetNode(const FZoneGraphTrafficLaneData* Lane);
	// Returns the lane with the lowest TotalCost in current OpenList and removes it 
	const FZoneGraphTrafficLaneData* PopCheapest();
	// Check lane connections and update OpenList according to A*
	void EvaluateLane(const FZoneGraphTrafficLaneData* Lane, const FZoneGraphTrafficLaneData* To);

	static float CalculatePathLength(const FTrafficPath& TrafficPath);
	
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	TWeakObjectPtr<const UMassTrafficSettings> MassTrafficSettings = nullptr;
	TObjectPtr<UMassTrafficSubsystem> MassTrafficSubsystem = nullptr;
	TObjectPtr<UZoneGraphSubsystem> ZoneGraphSubsystem = nullptr;

	TMap<const FZoneGraphTrafficLaneData*, FLaneNode> LaneNodes;
	TArray<const FZoneGraphTrafficLaneData*> OpenList;
	uint32 CurrentSearchIndex = 0;

	FTrafficPath CurrentPath;
	FZoneGraphLaneLocation CurrLocation;
	int32 LanePathIndex = -1;
	float LastValidDistanceAlongLane = 0;

	FVector LastTargetPosition = FVector::ZeroVector;
	FQuat LastTargetOrientation = FQuat::Identity;;
};
