#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include <Engine/TextureRenderTarget2D.h>
#include "VertexColorIntermediateActor.generated.h"

UCLASS()
class PATITITILINGEDITOR_API AVertexColorIntermediateActor : public AGeneratedDynamicMeshActor
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AStaticMeshActor* InStaticMeshActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TessellationNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TileNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RandomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int UScaling = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int VScaling = 1;



/*	UFUNCTION(BlueprintCallable, CallInEditor)*/
	void GenerateTexture();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTextureRenderTarget2D* OutputTexture;

	UFUNCTION(BlueprintCallable, CallInEditor)
	void Rebuild();

	void RemappingUV(UDynamicMesh* TargetMesh, float UScalling, float VScalling);


	int TriangleNum;
	TArray<FVector2f> TriangleVerticesUV;
	TArray<FVector4f> TriangleVerticesRGBA;
};