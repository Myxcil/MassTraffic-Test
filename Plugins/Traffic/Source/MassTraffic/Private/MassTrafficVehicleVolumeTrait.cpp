// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficVehicleVolumeTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "MassTrafficSubsystem.h"


UMassTrafficVehicleVolumeTrait::UMassTrafficVehicleVolumeTrait(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UMassTrafficVehicleVolumeTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	UMassTrafficSubsystem* MassTrafficSubsystem = UWorld::GetSubsystem<UMassTrafficSubsystem>(&World);
	check(MassTrafficSubsystem || BuildContext.IsInspectingData());

	// Add parameters as shared fragment
	const FConstSharedStruct ParamsSharedFragment = EntityManager.GetOrCreateConstSharedFragment(Params);
	BuildContext.AddConstSharedFragment(ParamsSharedFragment);

	// Add radius
	FAgentRadiusFragment& RadiusFragment = BuildContext.AddFragment_GetRef<FAgentRadiusFragment>();
	RadiusFragment.Radius = FMath::Max(Params.HalfLength,Params.HalfWidth);
}
