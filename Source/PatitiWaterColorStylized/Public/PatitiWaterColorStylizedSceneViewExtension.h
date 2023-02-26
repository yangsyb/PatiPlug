#pragma once
#include "SceneViewExtension.h"
#include "RHI.h"
#include "RHIResources.h"
#include <../Private/ScreenPass.h>

class UPatitiWaterColorStylizedSubsystem;
class UMaterialInterface;
class FRDGTexture;

class FPatitiWaterColorStylizedSceneViewExtension : public FSceneViewExtensionBase
{
public:
	FPatitiWaterColorStylizedSceneViewExtension(const FAutoRegister& AutoRegister, UPatitiWaterColorStylizedSubsystem* InWorldSubsystem);

	//~ Begin FSceneViewExtensionBase Interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView, const FRenderTargetBindingSlots& RenderTargets, TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) override;
	virtual void PostRenderBasePassMobile_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
	//~ End FSceneViewExtensionBase Interface

	void RenderNoise(FRDGBuilder& GraphBuilder, const FSceneView& View, const FIntRect& PrimaryViewRect, FGlobalShaderMap* GlobalShaderMap, FRHIBlendState* DefaultBlendState);
	void RenderKuwaharaFilter(FRDGBuilder& GraphBuilder, const FSceneView& View, const FIntRect& PrimaryViewRect, FScreenPassRenderTarget& SceneColorRenderTarget, FScreenPassRenderTarget& SceneDepthRenderTarget, FScreenPassRenderTarget& KuwaharaRenderTarget, FGlobalShaderMap* GlobalShaderMap, FRHIBlendState* DefaultBlendState);

private:
	UPatitiWaterColorStylizedSubsystem* StylizedWorldSubsystem;
};
