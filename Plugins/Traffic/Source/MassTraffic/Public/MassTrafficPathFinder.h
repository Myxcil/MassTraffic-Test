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

	bool SearchPath(const FVector& End);

	void InitPathFollowing();
	bool UpdatePathFollowing(const float LookAheadDistance, FVector& TargetPosition, FQuat& TargetOrientation);
	bool HasPath() const { return LanePath.IsValidIndex(LanePathIndex); }

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	const FZoneGraphLaneLocation& GetOrigin() const { return Origin; }
	const FZoneGraphLaneLocation& GetDestination() const { return Destination; }
	
	const FZoneGraphStorage* GetZoneGraphStorage(const FZoneGraphLaneHandle& LaneHandle) const;

	const FZoneGraphLaneLocation& GetCurrentLocation() const { return CurrLocation; }
	const FZoneGraphTrafficLaneData* GetCurrentLane() const;
	const FZoneGraphTrafficLaneData* GetNextLane() const;

	bool GetLastLaneLocation(FVector& Location) const { Location =  CurrLocation.Position; return CurrLocation.IsValid(); }
	float UpdateLaneLength(const FZoneGraphTrafficLaneData* CurrLane) const;
	float GetDistanceToNextLane() const;

	void SetEmergencyLane(const FZoneGraphLaneHandle& LaneHandle, const bool bIsEmergencyLane);

	DECLARE_DELEGATE_TwoParams(FLaneChanged, const FZoneGraphLaneHandle&, const FZoneGraphLaneHandle&);
	FLaneChanged OnLaneChanged;
	
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
#if WITH_EDITOR
	const TArray<const FZoneGraphTrafficLaneData*>& GetLastCalculatedPath() const { return LanePath; }
	const FColor& GetPathDebugColor() const { return PathDebugColor; }
#endif

protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, meta=(Tooltip="Which zone graph tags should be used for path finding"))
	FZoneGraphTagFilter ZoneGraphTagFilter;
	UPROPERTY(EditDefaultsOnly, meta=(Tooltip="Needed to offset the distance along destination lane or car will stop too soon"))
	float DestinationLaneOffset = 400.0f;
#if WITH_EDITORONLY_DATA	
	UPROPERTY(EditAnywhere)
	FColor PathDebugColor = FColor::Yellow;
#endif

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

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	TWeakObjectPtr<const UMassTrafficSettings> MassTrafficSettings = nullptr;
	TObjectPtr<UMassTrafficSubsystem> MassTrafficSubsystem = nullptr;
	TObjectPtr<UZoneGraphSubsystem> ZoneGraphSubsystem = nullptr;

	FZoneGraphLaneLocation Origin;
	FZoneGraphLaneLocation Destination;
	TMap<const FZoneGraphTrafficLaneData*, FLaneNode> LaneNodes;
	TArray<const FZoneGraphTrafficLaneData*> OpenList;
	uint32 CurrentSearchIndex = 0;

	TArray<const FZoneGraphTrafficLaneData*> LanePath;
	FZoneGraphLaneLocation CurrLocation;
	int32 LanePathIndex = -1;
};
