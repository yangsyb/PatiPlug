#include "PatitiWaterColorStylizedSceneViewExtension.h"
#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "RendererInterface.h"
#include "SceneViewExtensionContext.h"
#include "PatitiWaterColorStylizedSceneViewExtension.h"
#include "DynamicResolutionState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Runtime/Renderer/Private/TranslucentRendering.h"
#include "RHI.h"
#include "Runtime/Renderer/Private/SceneRendering.h"
#include "Runtime/Renderer/Private/ScreenPass.h"
#include "SceneView.h"
#include "KuwaharaPS.h"
#include <PixelShaderUtils.h>

DECLARE_GPU_STAT_NAMED(KuwaharaFilter, TEXT("KuwaharaFilter"));
DECLARE_GPU_STAT_NAMED(ScreenNoise, TEXT("ScreenNoise"));

FScreenPassTextureViewportParameters GetTextureViewportParameters(const FScreenPassTextureViewport& InViewport)
{
	const FVector2f Extent(InViewport.Extent);
	const FVector2f ViewportMin(InViewport.Rect.Min.X, InViewport.Rect.Min.Y);
	const FVector2f ViewportMax(InViewport.Rect.Max.X, InViewport.Rect.Max.Y);
	const FVector2f ViewportSize = ViewportMax - ViewportMin;

	FScreenPassTextureViewportParameters Parameters;

	if (!InViewport.IsEmpty())
	{
		Parameters.Extent = FVector2f(Extent);
		Parameters.ExtentInverse = FVector2f(1.0f / Extent.X, 1.0f / Extent.Y);

		Parameters.ScreenPosToViewportScale = FVector2f(0.5f, -0.5f) * ViewportSize;
		Parameters.ScreenPosToViewportBias = (0.5f * ViewportSize) + ViewportMin;

		Parameters.ViewportMin = InViewport.Rect.Min;
		Parameters.ViewportMax = InViewport.Rect.Max;

		Parameters.ViewportSize = ViewportSize;
		Parameters.ViewportSizeInverse = FVector2f(1.0f / Parameters.ViewportSize.X, 1.0f / Parameters.ViewportSize.Y);

		Parameters.UVViewportMin = ViewportMin * Parameters.ExtentInverse;
		Parameters.UVViewportMax = ViewportMax * Parameters.ExtentInverse;

		Parameters.UVViewportSize = Parameters.UVViewportMax - Parameters.UVViewportMin;
		Parameters.UVViewportSizeInverse = FVector2f(1.0f / Parameters.UVViewportSize.X, 1.0f / Parameters.UVViewportSize.Y);

		Parameters.UVViewportBilinearMin = Parameters.UVViewportMin + 0.5f * Parameters.ExtentInverse;
		Parameters.UVViewportBilinearMax = Parameters.UVViewportMax - 0.5f * Parameters.ExtentInverse;
	}

	return Parameters;
}


FPatitiWaterColorStylizedSceneViewExtension::FPatitiWaterColorStylizedSceneViewExtension(const FAutoRegister& AutoRegister, UPatitiWaterColorStylizedSubsystem* InWorldSubsystem) : 
	FSceneViewExtensionBase(AutoRegister), StylizedWorldSubsystem(InWorldSubsystem)
{

}

void FPatitiWaterColorStylizedSceneViewExtension::PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView, const FRenderTargetBindingSlots& RenderTargets, TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures)
{
// 	const FSceneViewFamily& ViewFamily = *InView.Family;
// 	DynamicRenderScaling::TMap<float> UpperBounds = ViewFamily.GetScreenPercentageInterface()->GetResolutionFractionsUpperBound();
// 	const auto FeatureLevel = InView.GetFeatureLevel();
// 	const float ScreenPercentage = UpperBounds[GDynamicPrimaryResolutionFraction] * ViewFamily.SecondaryViewFraction;
// 
// 	const FIntRect PrimaryViewRect = static_cast<const FViewInfo&>(InView).ViewRect;
// 
// 	FScreenPassTexture SceneColor(SceneTextures->GetContents()->SceneColorTexture, PrimaryViewRect);
// 	FScreenPassTexture SceneDepth(SceneTextures->GetContents()->SceneDepthTexture, PrimaryViewRect);
// 
// 	if (!SceneColor.IsValid() || !SceneDepth.IsValid())
// 	{
// 		return;
// 	}
// 
// 	{
// 		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
// 		FRDGTextureDesc KuwaharaFilterTextureDesc = SceneColor.Texture->Desc;
// 		KuwaharaFilterTextureDesc.Format = PF_FloatRGBA;
// 		FLinearColor ClearColor(0., 0., 0., 0.);
// 		KuwaharaFilterTextureDesc.ClearValue = FClearValueBinding(ClearColor);
// 
// 		FRDGTexture* KuwaharaFilterTexture = GraphBuilder.CreateTexture(KuwaharaFilterTextureDesc, TEXT("KuwaharaFilterTexture"));
// 		FScreenPassRenderTarget KuwaharaFilterRenderTarget = FScreenPassRenderTarget(KuwaharaFilterTexture, SceneColor.ViewRect, ERenderTargetLoadAction::EClear);
// 
// 		FScreenPassRenderTarget SceneColorRenderTarget(SceneColor, ERenderTargetLoadAction::ELoad);
// 		const FScreenPassTextureViewport SceneColorTextureViewport(SceneColor);
// 		FScreenPassRenderTarget SceneDepthRenderTarget(SceneDepth, ERenderTargetLoadAction::ELoad);
// 		const FScreenPassTextureViewport SceneDepthTextureViewPort(SceneDepth);
// 
// 		FRHIBlendState* DefaultBlendState = FScreenPassPipelineState::FDefaultBlendState::GetRHI();
// 
// 		RDG_EVENT_SCOPE(GraphBuilder, "PatitiStylizedPass", SceneColorTextureViewport.Rect.Width(), SceneColorTextureViewport.Rect.Height());
// 		FRHISamplerState* PointClampSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
// 		const FScreenPassTextureViewportParameters SceneTextureViewportParams = GetTextureViewportParameters(SceneColorTextureViewport);
// 		FScreenPassTextureInput SceneTextureInput;
// 		{
// 			SceneTextureInput.Viewport = SceneTextureViewportParams;
// 			SceneTextureInput.Texture = SceneColorRenderTarget.Texture;
// 			SceneTextureInput.Sampler = PointClampSampler;
// 		}
// 		
// 		{
// 			FScopeLock RegionScopeLock(&StylizedWorldSubsystem->RegionAccessCriticalSection);
// 			RenderKuwaharaFilter(GraphBuilder, InView, PrimaryViewRect, SceneColorRenderTarget, SceneDepthRenderTarget, KuwaharaFilterRenderTarget, GlobalShaderMap, DefaultBlendState);
// 		}
// 
// 	}

}

void FPatitiWaterColorStylizedSceneViewExtension::PostRenderBasePassMobile_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}

struct FPostProcessingInputs
{
	TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures = nullptr;
	FRDGTextureRef ViewFamilyTexture = nullptr;
	FRDGTextureRef CustomDepthTexture = nullptr;
	FTranslucencyViewResourcesMap TranslucencyViewResourcesMap;

	void Validate() const
	{
		check(SceneTextures);
		check(ViewFamilyTexture);
		check(TranslucencyViewResourcesMap.IsValid());
	}
};

void FPatitiWaterColorStylizedSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	const FSceneViewFamily& ViewFamily = *View.Family;
	DynamicRenderScaling::TMap<float> UpperBounds = ViewFamily.GetScreenPercentageInterface()->GetResolutionFractionsUpperBound();
	const auto FeatureLevel = View.GetFeatureLevel();
	const float ScreenPercentage = UpperBounds[GDynamicPrimaryResolutionFraction] * ViewFamily.SecondaryViewFraction;

	const FIntRect PrimaryViewRect = static_cast<const FViewInfo&>(View).ViewRect;

	FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);
	FScreenPassTexture SceneDepth((*Inputs.SceneTextures)->SceneDepthTexture, PrimaryViewRect);

	if (!SceneColor.IsValid() || !SceneDepth.IsValid())
	{
		return;
	}

	{
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

		FLinearColor ClearColor(0., 0., 0., 0.);

		FRDGTextureDesc KuwaharaFilterTextureDesc = SceneColor.Texture->Desc;
		KuwaharaFilterTextureDesc.Format = PF_FloatRGBA;
		KuwaharaFilterTextureDesc.ClearValue = FClearValueBinding(ClearColor);

		FRDGTexture* KuwaharaFilterTexture = GraphBuilder.CreateTexture(KuwaharaFilterTextureDesc, TEXT("KuwaharaFilterTexture"));
		FScreenPassRenderTarget KuwaharaFilterRenderTarget = FScreenPassRenderTarget(KuwaharaFilterTexture, SceneColor.ViewRect, ERenderTargetLoadAction::EClear);

		FScreenPassRenderTarget SceneColorRenderTarget(SceneColor, ERenderTargetLoadAction::ELoad);
		const FScreenPassTextureViewport SceneColorTextureViewport(SceneColor);
		FScreenPassRenderTarget SceneDepthRenderTarget(SceneDepth, ERenderTargetLoadAction::ELoad);
		const FScreenPassTextureViewport SceneDepthTextureViewPort(SceneDepth);

		FRHIBlendState* DefaultBlendState = FScreenPassPipelineState::FDefaultBlendState::GetRHI();

		RDG_EVENT_SCOPE(GraphBuilder, "PatitiStylizedPass", SceneColorTextureViewport.Rect.Width(), SceneColorTextureViewport.Rect.Height());
//		FRHISamplerState* PointClampSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
//		const FScreenPassTextureViewportParameters SceneTextureViewportParams = GetTextureViewportParameters(SceneColorTextureViewport);
// 		FScreenPassTextureInput SceneTextureInput;
// 		{
// 			SceneTextureInput.Viewport = SceneTextureViewportParams;
// 			SceneTextureInput.Texture = SceneColorRenderTarget.Texture;
// 			SceneTextureInput.Sampler = PointClampSampler;
// 		}
		{
			FScopeLock RegionScopeLock(&StylizedWorldSubsystem->RegionAccessCriticalSection);
			RenderKuwaharaFilter(GraphBuilder, View, PrimaryViewRect, SceneColorRenderTarget, SceneDepthRenderTarget, KuwaharaFilterRenderTarget, GlobalShaderMap, DefaultBlendState);
		}

	}
}

void FPatitiWaterColorStylizedSceneViewExtension::RenderNoise(FRDGBuilder& GraphBuilder, const FSceneView& View, const FIntRect& PrimaryViewRect, FGlobalShaderMap* GlobalShaderMap, FRHIBlendState* DefaultBlendState)
{
	SCOPED_GPU_STAT(GraphBuilder.RHICmdList, ScreenNoise);

}

void FPatitiWaterColorStylizedSceneViewExtension::RenderKuwaharaFilter(FRDGBuilder& GraphBuilder, const FSceneView& View, const FIntRect& PrimaryViewRect, FScreenPassRenderTarget& SceneColorRenderTarget, FScreenPassRenderTarget& SceneDepthRenderTarget, FScreenPassRenderTarget& KuwaharaRenderTarget, FGlobalShaderMap* GlobalShaderMap, FRHIBlendState* DefaultBlendState)
{
	SCOPED_GPU_STAT(GraphBuilder.RHICmdList, KuwaharaFilter);
	FRHIDepthStencilState* DepthStencilState = FScreenPassPipelineState::FDefaultDepthStencilState::GetRHI();

	FIntRect Viewport;
	Viewport = PrimaryViewRect;
	const FScreenPassTextureViewport ScreenViewport(SceneColorRenderTarget.Texture, Viewport);

	FKuwaharaPS::FParameters* KuwaharaParameters = GraphBuilder.AllocParameters<FKuwaharaPS::FParameters>();
	KuwaharaParameters->View = View.ViewUniformBuffer;
 	KuwaharaParameters->InputSceneTexture = SceneColorRenderTarget.Texture;
	KuwaharaParameters->InputDepthTexture = SceneDepthRenderTarget.Texture;
 	//KuwaharaParameters->InputSampler = TStaticSamplerState<>::GetRHI();
	//KuwaharaParameters->InputSampler = BilinearSampler;
	KuwaharaParameters->InputSampler = TStaticSamplerState<SF_Trilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
 	KuwaharaParameters->RenderTargets[0] = KuwaharaRenderTarget.GetRenderTargetBinding();
	KuwaharaParameters->Width = SceneColorRenderTarget.Texture->Desc.Extent.X;
	KuwaharaParameters->Height = SceneColorRenderTarget.Texture->Desc.Extent.Y;

	TShaderMapRef<FKuwaharaVS> KuwaharaVertexShader(GlobalShaderMap);
	TShaderMapRef<FKuwaharaPS> KuwaharaPixelShader(GlobalShaderMap);

 	GraphBuilder.AddPass(
 		RDG_EVENT_NAME("KuwaharaFilter"),
		KuwaharaParameters,
		ERDGPassFlags::Raster,
		[&View, KuwaharaVertexShader, KuwaharaPixelShader, KuwaharaParameters, ScreenViewport, DefaultBlendState, DepthStencilState](FRHICommandList& RHICmdList)
		{
			DrawScreenPass(
				RHICmdList,
				static_cast<const FViewInfo&>(View),
				ScreenViewport,
				ScreenViewport,
				FScreenPassPipelineState(KuwaharaVertexShader, KuwaharaPixelShader, DefaultBlendState, DepthStencilState),
				EScreenPassDrawFlags::None,
				[&](FRHICommandList&)
				{
					SetShaderParameters(RHICmdList, KuwaharaPixelShader, KuwaharaPixelShader.GetPixelShader(), *KuwaharaParameters);
				});
		}
 		);

// 	FPixelShaderUtils::AddFullscreenPass(
// 		GraphBuilder,
// 		GlobalShaderMap,
// 		RDG_EVENT_NAME("Test"),
// 		KuwaharaPixelShader,
// 		KuwaharaParameters,
// 		Viewport);

	FCopyRectPS::FParameters* Parameters = GraphBuilder.AllocParameters<FCopyRectPS::FParameters>();
	Parameters->InputTexture = KuwaharaRenderTarget.Texture;
	Parameters->InputSampler = TStaticSamplerState<>::GetRHI();
	Parameters->RenderTargets[0] = SceneColorRenderTarget.GetRenderTargetBinding();

	TShaderMapRef<FScreenPassVS> CopyVertexShader(GlobalShaderMap);
	TShaderMapRef<FCopyRectPS> CopyPixelShader(GlobalShaderMap);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("CopyToViewport"),
		Parameters,
		ERDGPassFlags::Raster,
		[&View, CopyVertexShader, CopyPixelShader, Parameters, ScreenViewport, DefaultBlendState](FRHICommandList& RHICmdList)
		{
			DrawScreenPass(
				RHICmdList,
				static_cast<const FViewInfo&>(View),
				ScreenViewport,
				ScreenViewport,
				FScreenPassPipelineState(CopyVertexShader, CopyPixelShader, DefaultBlendState),
				EScreenPassDrawFlags::None,
				[&](FRHICommandList&)
				{
					SetShaderParameters(RHICmdList, CopyPixelShader, CopyPixelShader.GetPixelShader(), *Parameters);
				});
		});



}
