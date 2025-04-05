// Copyright Epic Games, Inc. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FComponentVisualizer;
// Logs
DECLARE_LOG_CATEGORY_EXTERN(LogMassTrafficEditor, Log, All);

// CVars
extern int32 GDebugMassTrafficEditor;

class FMassTrafficEditorModule : public IModuleInterface
{
public:
	void RegisterComponentVisualizer(const FName& ComponentClassName, const TSharedPtr<FComponentVisualizer>& Visualizer);

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	TArray<FName> RegisteredComponentClassNames;

};
