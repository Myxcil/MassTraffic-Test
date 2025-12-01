// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassTrafficPathFinder.h"
#include "MassTrafficPathFollower.generated.h"

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
class MASSTRAFFIC_API UMassTrafficPathFollower : public UActorComponent
{
	GENERATED_BODY()

public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
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
	
	bool GetRandomLocation(FVector& Position) const;

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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	TWeakObjectPtr<const UMassTrafficSettings> MassTrafficSettingsPtr = nullptr;

	TWeakObjectPtr<UMassTrafficSubsystem> MassTrafficSubsystemPtr = nullptr;
	TWeakObjectPtr<UZoneGraphSubsystem> ZoneGraphSubsystemPtr = nullptr;

	FMassTrafficPathFinder PathFinder;
	
	FTrafficPath CurrentPath;
	FZoneGraphLaneLocation CurrLocation;
	int32 LanePathIndex = -1;
	float LastValidDistanceAlongLane = 0;

	FVector LastTargetPosition = FVector::ZeroVector;
	FQuat LastTargetOrientation = FQuat::Identity;;
};
