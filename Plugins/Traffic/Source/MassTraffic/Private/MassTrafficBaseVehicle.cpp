// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficBaseVehicle.h"

#include "MassAgentComponent.h"
#include "MassTrafficVehicleComponent.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
AMassTrafficBaseVehicle::AMassTrafficBaseVehicle(const FObjectInitializer& ObjectInitializer) : AActor(ObjectInitializer)
{
	CreateDefaultSubobject<UMassAgentComponent>(TEXT("MassAgent"));
	MassTrafficVehicleComponent = CreateDefaultSubobject<UMassTrafficVehicleComponent>(TEXT("MassTrafficVehicle"));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficBaseVehicle::PrepareForPooling_Implementation()
{
	SetActorEnableCollision(false);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficBaseVehicle::PrepareForGame_Implementation()
{
	SetActorEnableCollision(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficBaseVehicle::ApplyWheelMotionBlurNative(const TArray<UMaterialInstanceDynamic*>& MotionBlurMIDs)
{
	static FName NAME_Angle(TEXT("Angle"));
	
	if (CachedMotionBlurWheelAngle.Num() < MotionBlurMIDs.Num())
	{
		CachedMotionBlurWheelAngle.AddZeroed(MotionBlurMIDs.Num() - CachedMotionBlurWheelAngle.Num());

		for (int Wheel = 0; Wheel < MotionBlurMIDs.Num(); Wheel++)
		{
			if (UMaterialInstanceDynamic* MID = MotionBlurMIDs[Wheel])
			{
				MID->SetScalarParameterValue(NAME_Angle, 0.f);
			}
		}
	}

	if (CachedMotionBlurWheelMIDs.Num() < MotionBlurMIDs.Num())
	{
		CachedMotionBlurWheelMIDs.AddZeroed(MotionBlurMIDs.Num() - CachedMotionBlurWheelMIDs.Num());
	}

	for (int I = 0; I < MotionBlurMIDs.Num(); I++)
	{
		if (MassTrafficVehicleComponent->WheelAngularVelocities.IsValidIndex((I)))
		{
			if (UMaterialInstanceDynamic* MID = MotionBlurMIDs[I])
			{
				const float AbsAngularVelocity = FMath::RadiansToDegrees(FMath::Abs(MassTrafficVehicleComponent->WheelAngularVelocities[I]));
				float WheelAngle = AbsAngularVelocity / BlurAngleVelocityMax;
				WheelAngle = FMath::Clamp(WheelAngle, 0.f, 1.f) * BlurAngleMax;

				if (FMath::Abs(CachedMotionBlurWheelAngle[I] - WheelAngle) > KINDA_SMALL_NUMBER)
				{
					MID->SetScalarParameterValue(NAME_Angle, WheelAngle);
					CachedMotionBlurWheelAngle[I] = WheelAngle;
					CachedMotionBlurWheelMIDs[I] = MID;
				}
			}
		}
	}
}