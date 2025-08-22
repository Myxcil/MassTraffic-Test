// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficLightVisualizationProcessor.h"
#include "MassTrafficLights.h"
#include "MassTrafficSubsystem.h"
#include "MassExecutionContext.h"
#include "MassActorSubsystem.h"
#include "MassLODCollectorProcessor.h"
#include "MassRepresentationSubsystem.h"
#include "VisualLogger/VisualLogger.h"
#include "Components/MeshComponent.h"


FMassTrafficLightInstanceCustomData::FMassTrafficLightInstanceCustomData(const EMassTrafficLightStateFlags TrafficLightStateFlags)
{
	// Pack TrafficLightStateFlags into the least significant 8 bits of PackedParam1
	*reinterpret_cast<uint32*>(&PackedParam1) = static_cast<uint8>(TrafficLightStateFlags); 
}


FMassTrafficLightInstanceCustomData::FMassTrafficLightInstanceCustomData(const bool VehicleGo, const bool VehiclePrepareToStop, const bool PedestrianGo_FrontSide, const bool PedestrianGo_LeftSide, const bool PedestrianGo_RightSide)
{
	EMassTrafficLightStateFlags TrafficLightStateFlags = EMassTrafficLightStateFlags::None;
	if (VehicleGo)
	{
		TrafficLightStateFlags |= EMassTrafficLightStateFlags::VehicleGo;
	}
	if (VehiclePrepareToStop)
	{
		TrafficLightStateFlags |= EMassTrafficLightStateFlags::VehiclePrepareToStop;
	}
	if (PedestrianGo_FrontSide)
	{
		TrafficLightStateFlags |= EMassTrafficLightStateFlags::PedestrianGo_FrontSide;
	}
	if (PedestrianGo_LeftSide)
	{
		TrafficLightStateFlags |= EMassTrafficLightStateFlags::PedestrianGo_LeftSide;
	}
	if (PedestrianGo_RightSide)
	{
		TrafficLightStateFlags |= EMassTrafficLightStateFlags::PedestrianGo_RightSide;
	}

	// Pack TrafficLightStateFlags into the least significant 8 bits of PackedParam1
	*reinterpret_cast<uint32*>(&PackedParam1) = static_cast<uint8>(TrafficLightStateFlags);
}

UMassTrafficLightVisualizationProcessor::UMassTrafficLightVisualizationProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
	ExecutionOrder.ExecuteInGroup = UE::MassTraffic::ProcessorGroupNames::TrafficIntersectionVisualization;
	ExecutionOrder.ExecuteAfter.Add(UMassTrafficIntersectionVisualizationLODProcessor::StaticClass()->GetFName());
}

void UMassTrafficLightVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	EntityQuery.AddRequirement<FMassTrafficIntersectionFragment>(EMassFragmentAccess::ReadOnly);
}

UMassTrafficLightUpdateCustomVisualizationProcessor::UMassTrafficLightUpdateCustomVisualizationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true; // due to ReadWrite access to FMassRepresentationSubsystemSharedFragment
	ExecutionOrder.ExecuteInGroup = UE::MassTraffic::ProcessorGroupNames::TrafficIntersectionVisualization;
	ExecutionOrder.ExecuteAfter.Add(UMassTrafficLightVisualizationProcessor::StaticClass()->GetFName());
}

void UMassTrafficLightUpdateCustomVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassTrafficIntersectionFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMassTrafficLightsParameters>();

	EntityQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
#if ENABLE_VISUAL_LOG
	EntityQuery.AddSubsystemRequirement<UMassTrafficSubsystem>(EMassFragmentAccess::ReadOnly);
#endif // ENABLE_VISUAL_LOG
}

void UMassTrafficLightUpdateCustomVisualizationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Visualize traffic lights at all?
	if (!GMassTrafficTrafficLights)
	{
		return;
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("Visual Updates")) 

		// Visualize entities
		EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
			check(RepresentationSubsystem);
			FMassInstancedStaticMeshInfoArrayView ISMInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();

			const FMassTrafficLightsParameters& TrafficLightsParams = Context.GetConstSharedFragment<FMassTrafficLightsParameters>();

			const TConstArrayView<FMassTrafficIntersectionFragment> TrafficIntersectionFragments = Context.GetFragmentView<FMassTrafficIntersectionFragment>(); 
			const TConstArrayView<FMassRepresentationLODFragment> VisualizationLODFragments = Context.GetFragmentView<FMassRepresentationLODFragment>();
			const TArrayView<FMassRepresentationFragment> VisualizationFragments = Context.GetMutableFragmentView<FMassRepresentationFragment>(); 
			const TArrayView<FMassActorFragment> ActorList = Context.GetMutableFragmentView<FMassActorFragment>();

			for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
			{
				const FMassTrafficIntersectionFragment& TrafficIntersectionFragment = TrafficIntersectionFragments[EntityIt]; 
				const FMassRepresentationLODFragment& VisualizationLODFragment = VisualizationLODFragments[EntityIt];
				const FMassRepresentationFragment& VisualizationFragment = VisualizationFragments[EntityIt];
				FMassActorFragment& ActorInfo = ActorList[EntityIt];

				AActor* Actor = ActorInfo.GetMutable();

				// We only support StaticMeshInstances for traffic lights.
				if(VisualizationFragment.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
				{
					// Visualize lights
					for (const FMassTrafficLight& TrafficLight : TrafficIntersectionFragment.TrafficLights)
					{
						check(TrafficLightsParams.TrafficLightTypesStaticMeshDescHandle.IsValidIndex(TrafficLight.TrafficLightTypeIndex));
						const FStaticMeshInstanceVisualizationDescHandle TrafficLightTypesStaticMeshDescHandle = TrafficLightsParams.TrafficLightTypesStaticMeshDescHandle[TrafficLight.TrafficLightTypeIndex];
						if (TrafficLightTypesStaticMeshDescHandle.ToIndex() != INDEX_NONE)
						{
							// Get world space transform
							FTransform IntersectionLightTransform(FRotator(0.0, TrafficLight.ZRotation, 0.0f), TrafficLight.Position);  

							// Prepare custom data
							const FMassTrafficLightInstanceCustomData PackedCustomData(TrafficLight.TrafficLightStateFlags);

							// Add instance with custom data 
							ISMInfo[TrafficLightTypesStaticMeshDescHandle.ToIndex()].AddBatchedTransform(Context.GetEntity(EntityIt)
								, IntersectionLightTransform, IntersectionLightTransform, VisualizationLODFragment.LODSignificance);
							ISMInfo[TrafficLightTypesStaticMeshDescHandle.ToIndex()].AddBatchedCustomData(PackedCustomData, VisualizationLODFragment.LODSignificance);

							// Debug
							#if WITH_MASSTRAFFIC_DEBUG
								if (GMassTrafficDebugVisualization)
								{
									DrawDebugPoint(World, IntersectionLightTransform.GetLocation() + FVector(50.0f, 0.0f, 200.0f), 10.0f, UE::MassLOD::LODColors[static_cast<uint8>(EMassRepresentationType::StaticMeshInstance)]);
								}
							#endif
						}
					}
				}
				else if (Actor)
				{
					int32 LightIndex = 0;
					Actor->ForEachComponent<UMeshComponent>(false, [&](UMeshComponent* TrafficLightMeshComponent)
					{
						const FMassTrafficLight& TrafficLight = TrafficIntersectionFragment.TrafficLights[LightIndex];

						// Set light mesh primitive data (SetCustomPrimitiveDataFloat will check itself to noop if the
						// data hasn't changed)
						const FMassTrafficLightInstanceCustomData PackedCustomData(TrafficLight.TrafficLightStateFlags);
						TrafficLightMeshComponent->SetCustomPrimitiveDataFloat(/*DataIndex*/1, PackedCustomData.PackedParam1);

						LightIndex++;
					});

					check(LightIndex == TrafficIntersectionFragment.TrafficLights.Num());
				}

			}
		});
	}

#if ENABLE_VISUAL_LOG

	// Debug draw current visualization
	if (GMassTrafficDebugVisualization)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("DebugDisplayVisualization")) 

		EntityQuery.ForEachEntityChunk(Context, [this, InWorld = EntityManager.GetWorld()](FMassExecutionContext& Context)
		{
			const UMassTrafficSubsystem* MassTrafficSubsystem = Context.GetSubsystem<UMassTrafficSubsystem>();

			const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
			const TArrayView<FMassRepresentationFragment> VisualizationList = Context.GetMutableFragmentView<FMassRepresentationFragment>();

			for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
			{
				const FTransformFragment& TransformFragment = TransformList[EntityIt];
				FMassRepresentationFragment& Visualization = VisualizationList[EntityIt];
				const int32 CurrentVisualIdx = static_cast<int32>(Visualization.CurrentRepresentation);

				if (Visualization.CurrentRepresentation != EMassRepresentationType::None || GMassTrafficDebugVisualization >= 2)
				{
					DrawDebugPoint(InWorld, TransformFragment.GetTransform().GetLocation() + FVector(50.0f, 0.0f, 200.0f), 10.0f, UE::MassLOD::LODColors[CurrentVisualIdx]);
				}

				if ((Visualization.CurrentRepresentation != EMassRepresentationType::None && GMassTrafficDebugVisualization >= 2) || GMassTrafficDebugVisualization >= 3)
				{
					UE_VLOG_LOCATION(MassTrafficSubsystem, TEXT("MassTraffic Traffic Light Vis"), Log, TransformFragment.GetTransform().GetLocation() + FVector(50.0f, 0.0f, 200.0f), /*Radius*/ 10.0f, UE::MassLOD::LODColors[CurrentVisualIdx], TEXT("%d"), CurrentVisualIdx);
				}
			}
		});
	}

#endif
}

//----------------------------------------------------------------------//
// UMassTrafficIntersectionVisualizationLODProcessor
//----------------------------------------------------------------------//
UMassTrafficIntersectionVisualizationLODProcessor::UMassTrafficIntersectionVisualizationLODProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	bAutoRegisterWithProcessingPhases = true;
	ExecutionOrder.ExecuteInGroup = UE::MassTraffic::ProcessorGroupNames::TrafficIntersectionVisualization;
	ExecutionOrder.ExecuteAfter.Reset();
	ExecutionOrder.ExecuteAfter.Add(UMassTrafficIntersectionLODCollectorProcessor::StaticClass()->GetFName());
}

void UMassTrafficIntersectionVisualizationLODProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);

	CloseEntityQuery.AddTagRequirement<FMassTrafficIntersectionTag>(EMassFragmentPresence::All);
	CloseEntityAdjustDistanceQuery.AddTagRequirement<FMassTrafficIntersectionTag>(EMassFragmentPresence::All);
	FarEntityQuery.AddTagRequirement<FMassTrafficIntersectionTag>(EMassFragmentPresence::All);
	DebugEntityQuery.AddTagRequirement<FMassTrafficIntersectionTag>(EMassFragmentPresence::All);
	FilterTag = FMassTrafficIntersectionTag::StaticStruct();
}

//----------------------------------------------------------------------//
// UMassTrafficIntersectionLODCollectorProcessor
//----------------------------------------------------------------------//
UMassTrafficIntersectionLODCollectorProcessor::UMassTrafficIntersectionLODCollectorProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionOrder.ExecuteInGroup = UE::MassTraffic::ProcessorGroupNames::TrafficIntersectionVisualization;
	ExecutionOrder.ExecuteAfter.Reset();
	ExecutionOrder.ExecuteAfter.Add(UE::MassTraffic::ProcessorGroupNames::FrameStart);
}

void UMassTrafficIntersectionLODCollectorProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);

	EntityQuery_VisibleRangeAndOnLOD.AddTagRequirement<FMassTrafficIntersectionTag>(EMassFragmentPresence::All);
	EntityQuery_VisibleRangeOnly.AddTagRequirement<FMassTrafficIntersectionTag>(EMassFragmentPresence::All);
	EntityQuery_OnLODOnly.AddTagRequirement<FMassTrafficIntersectionTag>(EMassFragmentPresence::All);
	EntityQuery_NotVisibleRangeAndOffLOD.AddTagRequirement<FMassTrafficIntersectionTag>(EMassFragmentPresence::All);
}
