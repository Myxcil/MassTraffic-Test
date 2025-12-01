// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficPathFollower.h"

#include "MassTrafficInterpolation.h"
#include "MassTrafficSubsystem.h"
#include "MassTrafficUtils.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficPathFollower::BeginPlay()
{
	Super::BeginPlay();

	MassTrafficSettingsPtr = GetDefault<UMassTrafficSettings>();

	MassTrafficSubsystemPtr = GetWorld()->GetSubsystem<UMassTrafficSubsystem>();
	check(MassTrafficSubsystemPtr.IsValid());

	ZoneGraphSubsystemPtr = GetWorld()->GetSubsystem<UZoneGraphSubsystem>();
	check(ZoneGraphSubsystemPtr.IsValid());

	const TIndirectArray<FMassTrafficZoneGraphData>& ZoneGraphDataArray = MassTrafficSubsystemPtr->GetTrafficZoneGraphData();
	if (ZoneGraphDataArray.Num() == 0)
	{
		UE_LOG(LogMassTraffic, Warning, TEXT("No Zonegraph in scene, deactivating PathFinder for %s"), *GetOwner()->GetName())
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}
	
	PathFinder.Init(MassTrafficSubsystemPtr.Get(), ZoneGraphSubsystemPtr.Get(), ZoneGraphTagFilter, LaneSearchRadius);
	PathFinder.FindNearestLane(GetOwner()->GetActorLocation(), LaneSearchRadius, CurrLocation);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficPathFollower::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficPathFollower::SearchShortestPath(const TArray<FVector>& Starts, const TArray<FVector>& Ends)
{
	CurrentPath.Reset();
	
	FTrafficPath TempPath;
	float MinLength = std::numeric_limits<float>::max();
	for(int32 StartIndex=0; StartIndex < Starts.Num(); ++StartIndex)
	{
		const FVector& Start = Starts[StartIndex];
		for (int EndIndex = 0; EndIndex < Ends.Num(); ++EndIndex)
		{
			const FVector& End = Ends[EndIndex];
			if (PathFinder.SearchPath(Start, End, TempPath))
			{
				if (TempPath.TotalLength < MinLength)
				{
					MinLength = TempPath.TotalLength;
					CurrentPath = MoveTemp(TempPath);
				}
			}
		}
	}
	return CurrentPath.IsValid();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficPathFollower::SearchPath(const FVector& Start, const FVector& End)
{
	CurrentPath.Reset();
	return PathFinder.SearchPath(Start, End, CurrentPath);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficPathFollower::InitPathFollowing()
{
	LanePathIndex = 0;
	CurrLocation = CurrentPath.Origin;
	LastValidDistanceAlongLane = CurrentPath.Origin.DistanceAlongLane;

	if (OnLaneChanged.IsBound())
	{
		OnLaneChanged.Execute(FZoneGraphLaneHandle(), CurrentPath.Origin.LaneHandle);
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficPathFollower::UpdatePathFollowing(const float LookAheadDistance, FVector& TargetPosition, FQuat& TargetOrientation)
{
	const int32 PrevLanePathIndex = LanePathIndex;
	const FTransform& Transform = GetOwner()->GetTransform();
	
	const FVector Location = Transform.GetLocation();
	PathFinder.FindNearestLane(Location, LaneSearchRadius, CurrLocation);

	// Done?
	if (CurrLocation.LaneHandle == CurrentPath.Destination.LaneHandle && CurrLocation.DistanceAlongLane >= CurrentPath.Destination.DistanceAlongLane)
	{
		if (OnLaneChanged.IsBound())
		{
			OnLaneChanged.Execute(CurrLocation.LaneHandle, FZoneGraphLaneHandle());
		}
		return false;
	}

	// if calculated position doesn't match path position anymore,
	// follow the path until we reach our current lane 
	const FZoneGraphTrafficLaneData* CurrLane = CurrentPath.Path[LanePathIndex];
	if (CurrLocation.LaneHandle.IsValid())
	{
		if (CurrLocation.LaneHandle != CurrLane->LaneHandle)
		{
			for(int32 I=LanePathIndex+1; I < CurrentPath.Path.Num(); ++I)
			{
				if (CurrentPath.Path[I]->LaneHandle == CurrLocation.LaneHandle)
				{
					LanePathIndex = I;
					CurrLane = CurrentPath.Path[LanePathIndex];
					break;
				}
			}
		}
	}
	else
	{
		const float Distance = FVector::Distance(Location, CurrentPath.Origin.Position);
		if (Distance < LaneSearchRadius)
		{
			CurrLocation = CurrentPath.Origin;
		}
	}

	// Update distance travelled only if we are still on our path
	if (CurrLocation.LaneHandle == CurrLane->LaneHandle)
	{
		LastValidDistanceAlongLane = CurrLocation.DistanceAlongLane;
	}

	int32 NextLaneIndex = INDEX_NONE;
	if (LanePathIndex < CurrentPath.Path.Num()-1)
	{
		NextLaneIndex = CurrentPath.Path[LanePathIndex+1]->LaneHandle.Index;
	}

	// notfiy that we changed lanes
	if (PrevLanePathIndex != LanePathIndex)
	{
		if (OnLaneChanged.IsBound())
		{
			OnLaneChanged.Execute(CurrentPath.Path[PrevLanePathIndex]->LaneHandle, CurrentPath.Path[LanePathIndex]->LaneHandle);
		}
	}

	FMassTrafficLaneSegment LaneSegment;
	UE::MassTraffic::InterpolatePositionAndOrientationAlongContinuousLanes(
		*GetZoneGraphStorage(CurrLane->LaneHandle),
		CurrLane->LaneHandle.Index,
		CurrLane->Length,
		NextLaneIndex,
		LastValidDistanceAlongLane + LookAheadDistance,
		ETrafficVehicleMovementInterpolationMethod::CubicBezier,
		LaneSegment,
		TargetPosition,
		TargetOrientation);

	LastTargetPosition = TargetPosition;
	LastTargetOrientation = TargetOrientation;
	
	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphTrafficLaneData* UMassTrafficPathFollower::GetCurrentLane() const
{
	if (!CurrLocation.IsValid())
		return nullptr;

	return PathFinder.GetLaneData(CurrLocation.LaneHandle);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphTrafficLaneData* UMassTrafficPathFollower::GetNextLane() const
{
	return LanePathIndex < CurrentPath.Path.Num()-1 ? CurrentPath.Path[LanePathIndex+1] : nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
float UMassTrafficPathFollower::CalculateActualLaneLength(const FZoneGraphTrafficLaneData* CurrLane) const
{
	if (CurrLane->LaneHandle == CurrentPath.Destination.LaneHandle)
	{
		return CurrentPath.Destination.DistanceAlongLane + DestinationLaneOffset;
	}
	return CurrLane->Length;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
float UMassTrafficPathFollower::GetDistanceToNextLane() const
{
	if (CurrLocation.IsValid() && CurrentPath.Path.IsValidIndex(LanePathIndex))
	{
		const FZoneGraphTrafficLaneData* CurrLane = CurrentPath.Path[LanePathIndex];
		return CurrLane->Length - CurrLocation.DistanceAlongLane; 		
	}
	return std::numeric_limits<float>::max();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void UMassTrafficPathFollower::SetEmergencyLane(const FZoneGraphLaneHandle& LaneHandle, const bool bIsEmergencyLane)
{
	if (UMassTrafficSubsystem* MassTrafficSubsystem = MassTrafficSubsystemPtr.Get())
	{
		if (FZoneGraphTrafficLaneData* TrafficLaneData = MassTrafficSubsystem->GetMutableTrafficLaneData(LaneHandle))
		{
			TrafficLaneData->bIsEmergencyLane = bIsEmergencyLane;
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool UMassTrafficPathFollower::GetRandomLocation(FVector& Position) const
{
	return PathFinder.GetRandomLocation(Position);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
const FZoneGraphStorage* UMassTrafficPathFollower::GetZoneGraphStorage(const FZoneGraphLaneHandle& LaneHandle) const
{
	return ZoneGraphSubsystemPtr->GetZoneGraphStorage(LaneHandle.DataHandle);
}

