// (c) 2024 by Crenetic GmbH Studios

#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "CoreMinimal.h"
#include "ZoneGraphTypes.h"
#include "Components/SceneComponent.h"
#include "MassTrafficIntersectionComponent.generated.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
class UMassTrafficSubsystem;
class UZoneGraphSubsystem;
struct FMassTrafficZoneGraphData;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UENUM(BlueprintType)
enum class ETrafficIntersectionType : uint8
{
	PriorityRight,
	PriorityRoad,
	TrafficLights,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
USTRUCT()
struct FTrafficLightSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bShow = false;
	UPROPERTY(EditAnywhere)
	TArray<int32> OpenLanes;
	UPROPERTY(EditAnywhere)
	float Duration = 0;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UENUM()
enum class ETrafficLightPhase : uint8
{
	Red,
	RedYellow,
	Green,
	Yellow,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASSTRAFFIC_API UMassTrafficIntersectionComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Intersection, meta=(AllowPrivateAccess=true))
	float IntersectionSize = 1000.0f;
	UPROPERTY(EditAnywhere, Category=Intersection, meta=(AllowPrivateAccess=true))
	ETrafficIntersectionType IntersectionType = ETrafficIntersectionType::PriorityRight;
	UPROPERTY(EditAnywhere, Category=Intersection, meta=(AllowPrivateAccess=true, EditCondition="IntersectionType == EIntersectionType::PriorityRoad", EditConditionHides))
	TArray<int32> PriorityRoadSides;
	UPROPERTY(EditAnywhere, Category=Intersection, meta=(AllowPrivateAccess=true, EditCondition="IntersectionType == EIntersectionType::TrafficLights", EditConditionHides))
	TArray<FTrafficLightSetup> TrafficLightSetups;
	
public:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	UMassTrafficIntersectionComponent();

	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UMassTrafficSubsystem* GetMassTrafficSubsystem() const { return MassTrafficSubsystem; }
	UZoneGraphSubsystem* GetZoneGraphSubsystem() const { return ZoneGraphSubsystem; }

	UFUNCTION(CallInEditor)
	void RefreshLanes();

	void SetEmergencyLane(const FZoneGraphLaneHandle& LaneHandle, const bool bIsEmergency);
	
#if WITH_EDITOR
	ETrafficIntersectionType GetIntersectionType() const { return IntersectionType; }
	float GetIntersectionSize() const { return IntersectionSize; }
	int32 GetNumSides() const { return IntersectionSides.Num(); }
	bool IsPrioritySide(const int32 SideIndex) const { return IntersectionSides[SideIndex].bHasPriority; }
	const TArray<FZoneGraphLaneHandle>& GetLaneHandles() const { return LaneHandles; }
	const TArray<int32>& GetSideLaneIndices(const int32 SideIndex) const { return IntersectionSides[SideIndex].LaneIndices; }
	const TArray<FTrafficLightSetup>& GetTrafficLightSetups() const { return TrafficLightSetups; }
#endif

	static UMassTrafficIntersectionComponent* FindIntersection(const FZoneGraphLaneHandle& LaneHandle);
	
protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	struct FIntersectionSide
	{
		bool bHasPriority : 1 = false;
		bool bIsOpen : 1 = false;
		FVector DirectionIntoIntersection = FVector(0,0,0);
		TArray<int32> LaneIndices;
	};

	struct FRoadLanes
	{
		FVector Position;
		TArray<int32> LaneIndices;
	};

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void SortSides();
	
	void OpenAllLanes();

	void HandlePriorityRight(const float DeltaTime);
	void HandlePriorityRoad(const float DeltaTime);
	void HandleTrafficLights(const float DeltaTime);
	
	void UpdateBlockingLanes();

	void ApplyLaneStatus();
	void ApplyTrafficLightStatus();
	
	bool IsVehicleApproaching(const int32 SideIndex) const;
	bool DoesSideContainLane(const FIntersectionSide& Side, const FZoneGraphLaneHandle& LaneHandle) const;

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	TObjectPtr<UMassTrafficSubsystem> MassTrafficSubsystem;
	TObjectPtr<UZoneGraphSubsystem> ZoneGraphSubsystem;

	TArray<FZoneGraphLaneHandle> LaneHandles;
	TArray<TArray<int32>> BlockingLaneIndices; // which lanes to block when indexed lane has vehicles
	TArray<FIntersectionSide> IntersectionSides;
	TArray<FRoadLanes> RoadLanes;
	int32 CurrentTrafficPhase = 0;
	float PhaseTimeRemaining = 0;
	bool bIsEmergencyLaneSet = false;
};

