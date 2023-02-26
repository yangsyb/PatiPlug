#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Object.h"
#include "Subsystems/EngineSubsystem.h"
#include "Engine/EngineBaseTypes.h"
#include "PatitiWaterColorStylizedSceneViewExtension.h"

// #if WITH_EDITOR
// #include "EditorUndoClient.h"
// #endif

#include "PatitiWaterColorStylizedSubsystem.generated.h"

// #if WITH_EDITOR
// class FPatitiStylizedEditorUndoClient : public FEditorUndoClient
// {
// 
// };
// #else
// class FPatitiStylizedEditorUndoClient
// {
// };
// #endif

UCLASS()
class UPatitiWaterColorStylizedSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

// #if WITH_EDITOR
// 	// FEditorUndoClient pure virtual methods.
// 	virtual void PostUndo(bool bSuccess) override { };
// 	virtual void PostRedo(bool bSuccess) override { }
// #endif

private:
	TSharedPtr< class FPatitiWaterColorStylizedSceneViewExtension, ESPMode::ThreadSafe > StylizedSceneViewExtension;

	FCriticalSection RegionAccessCriticalSection;

public:
	friend class FPatitiWaterColorStylizedSceneViewExtension;
};