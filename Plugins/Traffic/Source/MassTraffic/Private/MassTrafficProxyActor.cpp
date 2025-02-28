// Fill out your copyright notice in the Description page of Project Settings.


#include "MassTrafficProxyActor.h"

#include "EngineUtils.h"


// Sets default values
AMassTrafficProxyActor::AMassTrafficProxyActor()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;
	auto Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
}
