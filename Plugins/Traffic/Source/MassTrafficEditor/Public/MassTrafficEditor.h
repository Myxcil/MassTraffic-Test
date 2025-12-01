// Copyright Epic Games, Inc. All Rights Reserved.


#pragma once

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "CoreMinimal.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
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
