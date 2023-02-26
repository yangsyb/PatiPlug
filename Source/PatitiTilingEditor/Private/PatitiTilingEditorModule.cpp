#include "PatitiTilingEditorModule.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FPatitiTilingModule"

void FPatitiTilingEditorModule::StartupModule()
{
// 	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PatitiStylized"))->GetBaseDir(), TEXT("Shaders"));
// 	AddShaderSourceDirectoryMapping(TEXT("/PatitiStylized"), PluginShaderDir);
}

void FPatitiTilingEditorModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPatitiTilingEditorModule, PatitiStylized)