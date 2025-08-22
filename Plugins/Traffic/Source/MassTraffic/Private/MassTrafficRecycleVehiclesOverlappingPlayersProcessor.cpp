// Copyright Epic Games, Inc. All Rights Reserved.


#include "MassTrafficRecycleVehiclesOverlappingPlayersProcessor.h"
#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassRepresentationProcessor.h"
#include "MassSimulationLOD.h"
#include "MassTrafficFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassRepresentationActorManagement.h"


UMassTrafficRecycleVehiclesOverlappingPlayersProcessor::UMassTrafficRecycleVehiclesOverlappingPlayersProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false;
}

void UMassTrafficRecycleVehiclesOverlappingPlayersProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddTagRequirement<FMassTrafficVehicleTag>(EMassFragmentPresence::Any);
	EntityQuery.AddTagRequirement<FMassTrafficParkedVehicleTag>(EMassFragmentPresence::Any);
	EntityQuery.AddConstSharedRequirement<FMassRepresentationParameters>();
}

void UMassTrafficRecycleVehiclesOverlappingPlayersProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& ExecutionContext)
{
	const UWorld* World = GetWorld();
	UMassRepresentationSubsystem* MassRepresentationSubsystem = UWorld::GetSubsystem<UMassRepresentationSubsystem>(World);
	if (!MassRepresentationSubsystem)
	{
		return;
	}

	// Get all the player viewpoints. We'll use that as the player locations to make sure
	// we're not overlapping. Most likely 1 player.
	TArray<FVector, TInlineAllocator< 1 > > PlayerLocations;
	{
		FConstPlayerControllerIterator PlayerIterator = World->GetPlayerControllerIterator();
		for (; PlayerIterator; ++PlayerIterator)
		{
			APlayerController* PlayerController = PlayerIterator->Get();
			check(PlayerController);

			FVector PlayerLocation;
			FRotator PlayerRotation;
			PlayerController->GetPlayerViewPoint( PlayerLocation, PlayerRotation);
			PlayerLocations.Add(PlayerLocation);
		}
	}

	EntityQuery.ForEachEntityChunk(ExecutionContext,
		[&PlayerLocations, &MassRepresentationSubsystem](FMassExecutionContext& Context)
	{
		const TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassActorFragment> ActorFragments = Context.GetMutableFragmentView<FMassActorFragment>();
		const TArrayView<FMassRepresentationLODFragment> RepresentationLODFragments = Context.GetMutableFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FMassRepresentationFragment> VisualizationFragments = Context.GetMutableFragmentView<FMassRepresentationFragment>();

		const bool bIsParkedVehicle = Context.DoesArchetypeHaveTag<FMassTrafficParkedVehicleTag>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			// Making the radius 2x to give us some room. Defaulted to 3m for parked cars which don't have radius fragments.
			const float VehicleRadiusSquared = RadiusFragments.IsEmpty() ? FMath::Square(300.0f * 2) : FMath::Square(RadiusFragments[EntityIt].Radius * 2); 

			const FTransformFragment& TransformFragment = TransformFragments[EntityIt];
			bool bIsOverlappingPlayer = false;
			for(FVector PlayerLocation : PlayerLocations)
			{
				if (FVector::DistSquared(TransformFragment.GetTransform().GetLocation(), PlayerLocation) < VehicleRadiusSquared)
				{
					bIsOverlappingPlayer = true;
					break;
				}
			}

			// If we are overlapping the player, lets get rid of this vehicle.
			if (bIsOverlappingPlayer)
			{
				const FMassEntityHandle Entity = Context.GetEntity(EntityIt);
				FMassActorFragment& ActorFragment = ActorFragments[EntityIt];
				FMassRepresentationFragment& RepresentationFragment = VisualizationFragments[EntityIt];
				FMassRepresentationLODFragment& RepresentationLODFragment = RepresentationLODFragments[EntityIt];

				if (ActorFragment.IsValid())
				{
					const FMassEntityHandle MassAgent(Entity);
					UMassRepresentationActorManagement::ReleaseAnyActorOrCancelAnySpawning(*MassRepresentationSubsystem, MassAgent, ActorFragment, RepresentationFragment);
				}

				RepresentationLODFragment.LOD = EMassLOD::Off;
				RepresentationFragment.CurrentRepresentation = EMassRepresentationType::None;

				if (bIsParkedVehicle)
				{
					// We can safely destroy parked vehicles as they don't have references to other entities.
					// Traffic vehicles do and destroying them will crash the game.
					Context.Defer().DestroyEntity(Context.GetEntity(EntityIt));
				}
				else
				{
					// We recycle traffic vehicles back into the system instead of destroying them. This cleanly
					// handles resetting them and clearing any pointers to other entities they may have.
					Context.Defer().SwapTags<FMassTrafficVehicleTag, FMassTrafficRecyclableVehicleTag>(Entity);
				}
			}
		}
	});

}
