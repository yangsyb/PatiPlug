#include "PatitiWaterColorStylizedSubsystem.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

void UPatitiWaterColorStylizedSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	StylizedSceneViewExtension = FSceneViewExtensions::NewExtension<FPatitiWaterColorStylizedSceneViewExtension>(this);
}

void UPatitiWaterColorStylizedSubsystem::Deinitialize()
{
	StylizedSceneViewExtension.Reset();
	StylizedSceneViewExtension = nullptr;
}