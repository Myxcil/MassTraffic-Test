// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficVehicleSyncTrait.h"

#include "ChaosVehicleMovementComponent.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"


namespace FMassAgentTraitsHelper 
{
	template<typename T>
	T* AsComponent(UObject& Owner)
	{
		T* Component = nullptr;
		if (AActor* AsActor = Cast<AActor>(&Owner))
		{
			Component = AsActor->FindComponentByClass<T>();
		}
		else
		{
			Component = Cast<T>(&Owner);
		}

		UE_CVLOG_UELOG(Component == nullptr, &Owner, LogMass, Error, TEXT("Trying to extract %s from %s failed")
			, *T::StaticClass()->GetName(), *Owner.GetName());

		return Component;
	}
}

//----------------------------------------------------------------------//
//  UMassTrafficVehicleMovementSyncTrait
//----------------------------------------------------------------------//
void UMassTrafficVehicleMovementSyncTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FChaosVehicleMovementComponentWrapperFragment>();
	BuildContext.AddFragment<FMassVelocityFragment>();
	
	BuildContext.GetMutableObjectFragmentInitializers().Add([=](UObject& Owner, FMassEntityView& EntityView, const EMassTranslationDirection CurrentDirection)
		{
			if (UChaosVehicleMovementComponent* MovementComp = FMassAgentTraitsHelper::AsComponent<UChaosVehicleMovementComponent>(Owner))
			{
				FChaosVehicleMovementComponentWrapperFragment& ComponentFragment = EntityView.GetFragmentData<FChaosVehicleMovementComponentWrapperFragment>();
				ComponentFragment.Component = MovementComp;

				FMassVelocityFragment& VelocityFragment = EntityView.GetFragmentData<FMassVelocityFragment>();
				VelocityFragment.Value = MovementComp->Velocity;
			}
		});

	if (EnumHasAnyFlags(SyncDirection, EMassTranslationDirection::ActorToMass) 
		|| BuildContext.IsInspectingData())
	{
		BuildContext.AddTranslator<UMassTrafficVehicleMovementToMassTranslator>();
	}
}


//----------------------------------------------------------------------//
//  UMassTrafficVehicleOrientationSyncTrait
//----------------------------------------------------------------------//
void UMassTrafficVehicleOrientationSyncTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FChaosVehicleMovementComponentWrapperFragment>();

	if (EnumHasAnyFlags(SyncDirection, EMassTranslationDirection::ActorToMass) 
		|| BuildContext.IsInspectingData())
	{
		BuildContext.AddTranslator<UMassTrafficVehicleOrientationToMassTranslator>();
	}
}

//----------------------------------------------------------------------//
//  UMassTrafficVehicleOrientationToMassTranslator
//----------------------------------------------------------------------//
UMassTrafficVehicleMovementToMassTranslator::UMassTrafficVehicleMovementToMassTranslator()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorldToMass;
	RequiredTags.Add<FChaosVehicleMovementCopyToMassTag>();
	bRequiresGameThreadExecution = true;
}

void UMassTrafficVehicleMovementToMassTranslator::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FChaosVehicleMovementComponentWrapperFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassTrafficVehicleMovementToMassTranslator::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FChaosVehicleMovementComponentWrapperFragment> ComponentList = Context.GetFragmentView<FChaosVehicleMovementComponentWrapperFragment>();
		const TArrayView<FTransformFragment> LocationList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

		const int32 NumEntities = Context.GetNumEntities();
		
		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (const UChaosVehicleMovementComponent* AsMovementComponent = ComponentList[i].Component.Get())
			{
				LocationList[i].GetMutableTransform().SetLocation(AsMovementComponent->GetActorNavLocation());
				
				VelocityList[i].Value = AsMovementComponent->Velocity;
			}
		}
	});
}

//----------------------------------------------------------------------//
//  UMassTrafficVehicleOrientationToMassTranslator
//----------------------------------------------------------------------//
UMassTrafficVehicleOrientationToMassTranslator::UMassTrafficVehicleOrientationToMassTranslator() :
	EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorldToMass;
	RequiredTags.Add<FChaosVehicleMovementCopyToMassTag>();
	bRequiresGameThreadExecution = true;
}

void UMassTrafficVehicleOrientationToMassTranslator::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FChaosVehicleMovementComponentWrapperFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassTrafficVehicleOrientationToMassTranslator::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk( Context, [this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FChaosVehicleMovementComponentWrapperFragment> ComponentList = Context.GetFragmentView<FChaosVehicleMovementComponentWrapperFragment>();
		const TArrayView<FTransformFragment> LocationList = Context.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = Context.GetNumEntities();
		
		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (const UChaosVehicleMovementComponent* AsMovementComponent = ComponentList[i].Component.Get())
			{
				if (AsMovementComponent->UpdatedComponent != nullptr)
				{
					LocationList[i].GetMutableTransform().SetRotation(AsMovementComponent->UpdatedComponent->GetComponentTransform().GetRotation());
				}
			}
		}
	});
}
