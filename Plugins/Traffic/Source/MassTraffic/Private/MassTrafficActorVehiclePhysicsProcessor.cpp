// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassTrafficActorVehiclePhysicsProcessor.h"
#include "MassTraffic.h"
#include "MassTrafficFragments.h"
#include "MassExecutionContext.h"
#include "MassRepresentationFragments.h"
#include "MassTrafficVehicleControlInterface.h"
#include "MassTrafficVehicleControlProcessor.h"

UMassTrafficActorVehiclePhysicsProcessor::UMassTrafficActorVehiclePhysicsProcessor()
	: ChaosPhysicsVehiclesQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionOrder.ExecuteInGroup = UE::MassTraffic::ProcessorGroupNames::VehicleBehavior;
	ExecutionOrder.ExecuteAfter.Add(UE::MassTraffic::ProcessorGroupNames::FrameStart);
	ExecutionOrder.ExecuteAfter.Add(UE::MassTraffic::ProcessorGroupNames::PreVehicleBehavior);
	ExecutionOrder.ExecuteAfter.Add(UE::MassTraffic::ProcessorGroupNames::VehicleSimulationLOD);
	ExecutionOrder.ExecuteAfter.Add(UMassTrafficVehicleControlProcessor::StaticClass()->GetFName());
}

void UMassTrafficActorVehiclePhysicsProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ChaosPhysicsVehiclesQuery.AddTagRequirement<FMassTrafficVehicleTag>(EMassFragmentPresence::Any);
	ChaosPhysicsVehiclesQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	ChaosPhysicsVehiclesQuery.AddRequirement<FMassTrafficPIDVehicleControlFragment>(EMassFragmentAccess::ReadOnly);
	ChaosPhysicsVehiclesQuery.AddRequirement<FMassTrafficVehicleDamageFragment>(EMassFragmentAccess::ReadOnly);
	ChaosPhysicsVehiclesQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite);
	ChaosPhysicsVehiclesQuery.AddRequirement<FMassTrafficDebugFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
}

void UMassTrafficActorVehiclePhysicsProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Pass new vehicle control inputs to HighRes actors 
	ChaosPhysicsVehiclesQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ComponentSystemExecutionContext)
	{
		const TConstArrayView<FMassRepresentationFragment> RepresentationFragments = Context.GetFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassTrafficPIDVehicleControlFragment> PIDVehicleControlFragments = Context.GetFragmentView<FMassTrafficPIDVehicleControlFragment>();
		const TConstArrayView<FMassTrafficVehicleDamageFragment> VehicleDamageFragments = Context.GetFragmentView<FMassTrafficVehicleDamageFragment>();
		const TArrayView<FMassActorFragment> ActorFragments = Context.GetMutableFragmentView<FMassActorFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (RepresentationFragments[EntityIt].CurrentRepresentation != EMassRepresentationType::HighResSpawnedActor)
			{
				continue;
			}

			const FMassTrafficPIDVehicleControlFragment& PIDVehicleControlFragment = PIDVehicleControlFragments[EntityIt];
			FMassActorFragment& ActorFragment = ActorFragments[EntityIt];

			AActor* Actor = ActorFragment.GetMutable();
			if (Actor != nullptr && Actor->Implements<UMassTrafficVehicleControlInterface>())
			{
				if (VehicleDamageFragments[EntityIt].VehicleDamageState >= EMassTrafficVehicleDamageState::Totaled)
				{
					Context.Defer().PushCommand<FMassDeferredSetCommand>([Actor](FMassEntityManager&)
					{
						// Throttle off, full brake, no handbrake, steering is not set.
						IMassTrafficVehicleControlInterface::Execute_SetVehicleInputs(Actor, 0.0f, 1.0f, false, 0.0f, false);
					});
				}
				else
				{
					Context.Defer().PushCommand<FMassDeferredSetCommand>([Actor, PIDVehicleControlFragment](FMassEntityManager&)
					{
						IMassTrafficVehicleControlInterface::Execute_SetVehicleInputs(Actor,
							PIDVehicleControlFragment.Throttle,
							PIDVehicleControlFragment.Brake,
							PIDVehicleControlFragment.bHandbrake,
							PIDVehicleControlFragment.Steering,
							/*bSetSteering*/ true);
					});
				}
			}
		}
	});
}