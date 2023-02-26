#include "VertexColorIntermediateActor.h"
#include "VertexColorToTextureCS.h"
#include <RenderGraphUtils.h>
#include <RHI.h>
#include "GeometryScript/SceneUtilityFunctions.h"
#include "GeometryScript/MeshSubdivideFunctions.h"
#include "GeometryScript/MeshUVFunctions.h"
#include "GeometryScript/MeshVertexColorFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "Engine/StaticMeshActor.h"

AVertexColorIntermediateActor::AVertexColorIntermediateActor(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>();
	NewRenderTarget2D->RenderTargetFormat = RTF_RGBA16f;
	NewRenderTarget2D->ClearColor = FLinearColor::Green;
	NewRenderTarget2D->bAutoGenerateMips = false;
	NewRenderTarget2D->bCanCreateUAV = true;
	NewRenderTarget2D->InitAutoFormat(4096, 4096);
	NewRenderTarget2D->UpdateResourceImmediate(true);
	OutputTexture = NewRenderTarget2D;
}


void AVertexColorIntermediateActor::GenerateTexture()
{
	if (!TriangleVerticesUV.Num() || !TriangleVerticesRGBA.Num() || TriangleNum <= 0) return;
	TResourceArray<STVerticesUV> UVBuffer;
	TResourceArray<STVerticesRGBA> RGBABuffer;
	UVBuffer.SetNumUninitialized(TriangleNum * 3);
	RGBABuffer.SetNumUninitialized(TriangleNum * 3);
	for (int Index = 0; Index < TriangleNum * 3; Index++)
	{
		UVBuffer[Index].UV = TriangleVerticesUV[Index];
		RGBABuffer[Index].RGBA = TriangleVerticesRGBA[Index];
	}
	FRHIResourceCreateInfo UVCreateInfo(TEXT("UVBuffer"), &UVBuffer);
	FRHIResourceCreateInfo RGBACreateInfo(TEXT("RGBABuffer"), &RGBABuffer);

	FUnorderedAccessViewRHIRef OutputTextureUAV = RHICreateUnorderedAccessView(OutputTexture->TextureReference.TextureReferenceRHI->GetReferencedTexture(), 0);
	FBufferRHIRef InputVerticesUVStructuredBuffer = RHICreateStructuredBuffer(sizeof(STVerticesUV), sizeof(STVerticesUV) * TriangleNum * 3, BUF_ShaderResource, UVCreateInfo);
	FBufferRHIRef InputVerticesRGBAStructuredBuffer = RHICreateStructuredBuffer(sizeof(STVerticesRGBA), sizeof(STVerticesRGBA) * TriangleNum * 3, BUF_ShaderResource, RGBACreateInfo);
	FShaderResourceViewRHIRef InputTriangleVertexUVs = RHICreateShaderResourceView(InputVerticesUVStructuredBuffer);
	FShaderResourceViewRHIRef InputVerticesRGBASRV = RHICreateShaderResourceView(InputVerticesRGBAStructuredBuffer);
	ENQUEUE_RENDER_COMMAND(FComputeShaderRunner)(
		[&](FRHICommandListImmediate& RHICmdList)
		{
			TShaderMapRef<FVertexColorToTextureCS> VertexColorToTextureComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

			FVertexColorToTextureCS::FParameters Parameters;
			Parameters.OutputTexture = OutputTextureUAV;
			Parameters.InputTriangleVertexUVs = InputTriangleVertexUVs;
			Parameters.InputVerticesRGBA = InputVerticesRGBASRV;

			FComputeShaderUtils::Dispatch(RHICmdList, VertexColorToTextureComputeShader, Parameters, FIntVector(TriangleNum, 1, 1));
		}
	);
}

void AVertexColorIntermediateActor::Rebuild()
{
	if (!InStaticMeshActor) return;
	UDynamicMesh* TargetMesh = this->GetDynamicMeshComponent()->GetDynamicMesh();
	TargetMesh->Reset();
	TriangleVerticesUV.Empty();
	TriangleVerticesRGBA.Empty();
	this->OnRebuildGeneratedMesh(TargetMesh);
	
	FTransform LocalToWorld;
	TEnumAsByte<EGeometryScriptOutcomePins> OutCome;
	UGeometryScriptLibrary_SceneUtilityFunctions::CopyMeshFromComponent(InStaticMeshActor->GetStaticMeshComponent(), TargetMesh, FGeometryScriptCopyMeshFromComponentOptions(), false, LocalToWorld, OutCome, nullptr);
	UGeometryScriptLibrary_MeshSubdivideFunctions::ApplyUniformTessellation(TargetMesh, TessellationNum);
	RemappingUV(TargetMesh, 1.f / UScaling, 1.f / VScaling);
}

void AVertexColorIntermediateActor::RemappingUV(UDynamicMesh* TargetMesh, float UScalling, float VScalling)
{
	double MeshArea;
	double UVArea;
	FBox MeshBounds;
	FBox2D UVBounds;
	bool bIsValidUVSet;
	bool bFoundUnsetUVs;
	UGeometryScriptLibrary_MeshUVFunctions::GetMeshUVSizeInfo(TargetMesh, 0, FGeometryScriptMeshSelection(), MeshArea, UVArea, MeshBounds, UVBounds, bIsValidUVSet, bFoundUnsetUVs);
	FVector2D Translation = FVector2D(0, 0) - UVBounds.Min;
	UGeometryScriptLibrary_MeshUVFunctions::TranslateMeshUVs(TargetMesh, 0, Translation, FGeometryScriptMeshSelection());
	UGeometryScriptLibrary_MeshUVFunctions::GetMeshUVSizeInfo(TargetMesh, 0, FGeometryScriptMeshSelection(), MeshArea, UVArea, MeshBounds, UVBounds, bIsValidUVSet, bFoundUnsetUVs);
	FVector2D Scale = FVector2D(UScaling, VScalling) / UVBounds.Max;
	UGeometryScriptLibrary_MeshUVFunctions::ScaleMeshUVs(TargetMesh, 0, Scale, FVector2D(), FGeometryScriptMeshSelection());
	FGeometryScriptUVList UVList;
	FGeometryScriptColorList ColorList;
	bool IsValid;
	bool HasIDGap;
	bool HasSplitUVs;
	UGeometryScriptLibrary_MeshUVFunctions::GetMeshPerVertexUVs(TargetMesh, 0 , UVList, IsValid, HasIDGap, HasSplitUVs);
	UGeometryScriptLibrary_MeshVertexColorFunctions::GetMeshPerVertexColors(TargetMesh, ColorList, IsValid, HasIDGap);
	TriangleNum = TargetMesh->GetTriangleCount();
	for(int Index = 0; Index < TriangleNum; Index++)
	{
		bool IsValidTriangle;
		FIntVector Indices = UGeometryScriptLibrary_MeshQueryFunctions::GetTriangleIndices(TargetMesh, Index, IsValidTriangle);
		if(!IsValidTriangle) break;
		auto UVArray = *UVList.List.Get();
		auto ColorArray = *ColorList.List.Get();
		for(int VertexIndex = 0; VertexIndex < 3; VertexIndex++)
		{
			int CurrentIndex = Indices[VertexIndex];
			FVector2f UV;
			UV[0] = UVArray[CurrentIndex].X;
			UV[1] = UVArray[CurrentIndex].Y;
			TriangleVerticesUV.Add(UV);
			FVector4f RGBA;
			RGBA[0] = ColorArray[CurrentIndex].R;
			RGBA[1] = ColorArray[CurrentIndex].G;
			RGBA[2] = ColorArray[CurrentIndex].B;
			RGBA[3] = ColorArray[CurrentIndex].A;
			TriangleVerticesRGBA.Add(RGBA);
		}
	}
}
