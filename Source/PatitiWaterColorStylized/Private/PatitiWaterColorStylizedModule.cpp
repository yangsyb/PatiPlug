// Copyright Epic Games, Inc. All Rights Reserved.

#include "PatitiWaterColorStylizedModule.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FPatitiWaterColorStylizedModule"

void FPatitiWaterColorStylizedModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PatitiStylized"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/PatitiStylized"), PluginShaderDir);
}

void FPatitiWaterColorStylizedModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPatitiWaterColorStylizedModule, PatitiStylized)