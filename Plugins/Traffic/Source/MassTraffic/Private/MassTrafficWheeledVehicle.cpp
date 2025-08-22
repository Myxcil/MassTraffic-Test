// (c) 2024 by Crenetic GmbH Studios


#include "MassTrafficWheeledVehicle.h"

#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------------
AMassTrafficWheeledVehicle::AMassTrafficWheeledVehicle(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
bool AMassTrafficWheeledVehicle::CanBePooled_Implementation()
{
	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::PrepareForPooling_Implementation()
{
	DetachFromControllerPendingDestroy();

	if (UChaosVehicleMovementComponent* MovementComponent = GetVehicleMovement())
	{
		MovementComponent->ResetVehicle(),
		MovementComponent->StopMovementImmediately();
	}

	if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
	{
		SkeletalMeshComponent->SetSimulatePhysics(false);
	}
	
	SetActorEnableCollision(false);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::PrepareForGame_Implementation()
{
	SetActorEnableCollision(true);

	if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
	{
		SkeletalMeshComponent->SetSimulatePhysics(true);
		SkeletalMeshComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
		SkeletalMeshComponent->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	}

	// Reset any wheel motion blur
	for (int i = 0; i < CachedMotionBlurWheelMIDs.Num(); i++)
	{
		if (CachedMotionBlurWheelMIDs[i] && CachedMotionBlurWheelAngle.IsValidIndex(i) && CachedMotionBlurWheelAngle[i] != 0.0f)
		{
			static FName NAME_Angle(TEXT("Angle"));
			CachedMotionBlurWheelMIDs[i]->SetScalarParameterValue(NAME_Angle, 0.0f);
		}
	}
	CachedMotionBlurWheelMIDs.Empty();
	CachedMotionBlurWheelAngle.Empty();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::SetVehicleInputs_Implementation(const float Throttle, const float Brake, const bool bHandBrake, const float Steering, const bool bSetSteering)
{
	if (UChaosVehicleMovementComponent* MoveCmp = GetVehicleMovement())
	{
		MoveCmp->SetThrottleInput(Throttle);
		MoveCmp->SetBrakeInput(Brake);
		MoveCmp->SetHandbrakeInput(bHandBrake);
		if (bSetSteering)
		{
			MoveCmp->SetSteeringInput(Steering);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::OnParkedVehicleSpawned_Implementation()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::OnTrafficVehicleSpawned_Implementation()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::BeginPlay()
{
	Super::BeginPlay();

	UChaosWheeledVehicleMovementComponent* MoveCmp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
	check(MoveCmp);
	MoveCmp->SetRequiresControllerForInputs(false);
	ChaosMovementComponent = MoveCmp;

	AudioEngine = MoveCmp->GetOwner()->FindComponentByTag<UAudioComponent>("AudioEngine");
	if(AudioEngine.IsValid())
	{
		SoundEnabled = true;
		OnToggleMotor.AddUniqueDynamic(this, &ThisClass::ToggleEngineSound);
	}

	if(EngineStartSound)
	{
		EngineStartDelay = EngineStartSound->Duration;
	}
	if(EngineStartSound && EngineSound)
	{
		EngineLoopDelay = EngineStartSound->Duration + EngineSound->Duration;
	}

	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(SoundEnabled)
	{
		HandleVehicleSound();
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::ApplyWheelMotionBlurParameters(const TArray<UMaterialInstanceDynamic*> MotionBlurMIDs)
{
	static FName NAME_Angle(TEXT("Angle"));
	
	if (MotionBlurMIDs.IsEmpty())
		return;
	
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

	if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetMovementComponent()))
	{
		if (MoveComp->Wheels.Num() == MotionBlurMIDs.Num())
		{
			for (int i = 0; i < MotionBlurMIDs.Num(); i++)
			{
				if (UChaosVehicleWheel* Wheel = MoveComp->Wheels[i])
				{
					if (UMaterialInstanceDynamic* MID = MotionBlurMIDs[i])
					{
						const float AbsAngularVelocity = FMath::RadiansToDegrees(FMath::Abs(Wheel->GetWheelAngularVelocity()));
						float WheelAngle = AbsAngularVelocity / BlurAngleVelocityMax;
						WheelAngle = FMath::Clamp(WheelAngle, 0.f, 1.f) * BlurAngleMax;

						if (FMath::Abs(CachedMotionBlurWheelAngle[i] - WheelAngle) > KINDA_SMALL_NUMBER)
						{
							MID->SetScalarParameterValue(NAME_Angle, WheelAngle);
							CachedMotionBlurWheelAngle[i] = WheelAngle;
							CachedMotionBlurWheelMIDs[i] = MID;
						}
					}
				}
			}
		}
	}
}

//------------------------------------------------------------Sound----------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::HandleVehicleSound()
{
	if(IsMotorRunning())
		PlayEngineSound();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
void AMassTrafficWheeledVehicle::PlayEngineSound()
{
	float CurrentRPM = ChaosMovementComponent->GetEngineRotationSpeed() * 0.001f;
	float EngineFizz = UKismetMathLibrary::MapRangeClamped(ChaosMovementComponent->GetEngineRotationSpeed(), 900.f,ChaosMovementComponent->GetEngineMaxRotationSpeed(), 0, 12.f);
	
	// Update which Waveplayer we are hearing
	if (CurrentRPM < .85f)
	{
		AudioEngine->SetFloatParameter("600_Vol", 1);
		AudioEngine->SetFloatParameter("1100_Vol", 0);
		AudioEngine->SetFloatParameter("1700_Vol", 0);
		AudioEngine->SetFloatParameter("2200_Vol", 0);
	}
	else if (CurrentRPM < 1.1f)
	{		
		AudioEngine->SetFloatParameter("600_Vol", 0);
		AudioEngine->SetFloatParameter("1100_Vol", 1);
		AudioEngine->SetFloatParameter("1700_Vol", 0);
		AudioEngine->SetFloatParameter("2200_Vol", 0);
	}
	else if (CurrentRPM < 1.8f)
	{
		AudioEngine->SetFloatParameter("600_Vol", 0);
		AudioEngine->SetFloatParameter("1100_Vol", 0);
		AudioEngine->SetFloatParameter("1700_Vol", 1);
		AudioEngine->SetFloatParameter("2200_Vol", 0);
	}
	else
	{
		AudioEngine->SetFloatParameter("600_Vol", 0);
		AudioEngine->SetFloatParameter("1100_Vol", 0);
		AudioEngine->SetFloatParameter("1700_Vol", 0);
		AudioEngine->SetFloatParameter("2200_Vol", 1);
	}	
	
	// sound modulation
	AudioEngine->SetFloatParameter("PitchShift", CurrentRPM * 0.001f);
	AudioEngine->SetFloatParameter("EngFizz", EngineFizz);
}

void AMassTrafficWheeledVehicle::ToggleEngineSound(bool MotorState)
{
	// Trigger the correct rpm range sounds
	if (MotorState)
	{		
		AudioEngine->Play();
		AudioEngine->SetBoolParameter("Trig_Start", true);
		AudioEngine->SetFloatParameter("EngStartCF", EngineStartDelay);
		AudioEngine->SetFloatParameter("Trig_0-2_CF", EngineLoopDelay);
		AudioEngine->SetBoolParameter("Trig_0-2_Play", true);
		AudioEngine->SetFloatParameter("600_Vol", 1.0);				
	}
	else
	{
		AudioEngine->SetFloatParameter("EngStartCF", 0);		
		AudioEngine->SetBoolParameter("ResetEngDelay", true);
		AudioEngine->SetBoolParameter("Trig_Stop", true);
		AudioEngine->SetFloatParameter("600_Vol", 0);			
	}
	// Stop all the other rpm ranges
}
