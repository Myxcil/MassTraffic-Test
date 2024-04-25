// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "MassLODSubsystem.h"
#include "MassRepresentationTypes.h"

#include "MassTrafficLights.generated.h"

class UStaticMesh;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct MASSTRAFFIC_API FMassTrafficLightTypeData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStaticMeshInstanceVisualizationDesc StaticMeshInstanceDesc;

	/** This light is suitable for roads with this many lanes. 0 = Any */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumLanes = 0;
};

UCLASS(Blueprintable, BlueprintType)
class MASSTRAFFIC_API UMassTrafficLightTypesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(TitleProperty="Name"))
	TArray<FMassTrafficLightTypeData> TrafficLightTypes;
};

USTRUCT()
struct MASSTRAFFIC_API FMassTrafficLightInstanceDesc
{
	GENERATED_BODY()
	FMassTrafficLightInstanceDesc()
	{}

	FMassTrafficLightInstanceDesc(FVector InPosition, float InZRotation, FVector InControlledIntersectionMidpoint, uint8 InTrafficLightTypeIndex) :
		Position(InPosition),
		ZRotation(InZRotation),
		ControlledIntersectionSideMidpoint(InControlledIntersectionMidpoint),
		TrafficLightTypeIndex(InTrafficLightTypeIndex)
	{
	}

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	float ZRotation = 0.0f;

	UPROPERTY()
	FVector ControlledIntersectionSideMidpoint = FVector::ZeroVector;

	UPROPERTY()
	int16 TrafficLightTypeIndex = INDEX_NONE;
};

UCLASS(Blueprintable, BlueprintType)
class MASSTRAFFIC_API UMassTrafficLightInstancesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category="Traffic Lights")
	TObjectPtr<const UMassTrafficLightTypesDataAsset> TrafficLightTypesData = nullptr;
	
	/**
	 * Traffic lights to spawn
	 * @see PopulateTrafficLightsFromPointCloud
	 */
	UPROPERTY()
	TArray< FMassTrafficLightInstanceDesc > TrafficLights;
	
	/** Number of stored traffic light instances */
	UPROPERTY(VisibleAnywhere, Transient, Category="Traffic Lights", meta=(ArrayClamp))
	int32 NumTrafficLights;

#if WITH_EDITOR

	/** Generate traffic light data from current map */
	UFUNCTION(CallInEditor, Category="Point Cloud")
	void PopulateTrafficLightsFromMap();

	/** Clear / reset the TrafficLightDetails list */   
	UFUNCTION(CallInEditor, Category="Point Cloud")
	void ClearTrafficLights()
	{
		TrafficLights.Reset();
		NumTrafficLights = 0;
	}

#endif

	// UObject overrides
	virtual void PostLoad() override
	{
		Super::PostLoad();

		NumTrafficLights = TrafficLights.Num();
	}

	FRandomStream RandomStream;
};

USTRUCT(BlueprintType)
struct MASSTRAFFIC_API FMassTrafficLightsParameters : public FMassSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const UMassTrafficLightTypesDataAsset> TrafficLightTypesData = nullptr;
	
	UPROPERTY(Transient)
	TArray<FStaticMeshInstanceVisualizationDescHandle> TrafficLightTypesStaticMeshDescHandle;
};
