// (c) 2024 by Crenetic GmbH Studios

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Components/ActorComponent.h"
#include "MassTrafficTrackNearVehicles.generated.h"


class UMassActorSubsystem;
class AMassTrafficControlledVehicle;
class UMassTrafficPathFinder;
class UMassEntitySubsystem;

struct FNearestVehicleInfo
{
	FMassEntityHandle Handle;
	FVector Position;
	float Speed;
	float Distance;
	float TimeToCollision;
	float DistanceToCollision;

	void Reset()
	{
		Handle.Reset();
		Distance = TNumericLimits<float>::Max();
		TimeToCollision = TNumericLimits<float>::Max();
		DistanceToCollision = TNumericLimits<float>::Max();
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASSTRAFFIC_API UMassTrafficTrackNearVehicles : public UActorComponent
{
	GENERATED_BODY()

public:
	UMassTrafficTrackNearVehicles();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	const FNearestVehicleInfo& GetNearestVehicleInfo() const { return NearestVehicleInfo; }
	
private:
	void DetermineNearestVehicle();

	UPROPERTY(Transient)
	TObjectPtr<const AMassTrafficControlledVehicle> ControlledVehicle = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<const UMassEntitySubsystem> EntitySubsystem = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UMassActorSubsystem> MassActorSubsystem = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float HalfLength = 350.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float HalfWidth = 150.0f;

	FNearestVehicleInfo NearestVehicleInfo;
};
