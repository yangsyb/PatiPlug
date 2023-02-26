#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include <Engine/TextureRenderTarget2D.h>
#include "ScatterIntermediateActor.generated.h"

UCLASS()
class PATITITILINGEDITOR_API AScatterIntermediateActor : public AGeneratedDynamicMeshActor
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AStaticMeshActor* InStaticMeshActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RandomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int BrushNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ColorRangeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ColorRangeMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DecalSizeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DecalSizeMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UMaterialInterface*> BrushesMaterial;

	TArray<FTransform> BrushesTransform;
	TArray<float> BrushesSize;

	TArray<ABrushStrokeDecalActor*> GeneratedActors;

	UFUNCTION(BlueprintCallable, CallInEditor)
	void GenerateBrushes();

	UFUNCTION(BlueprintCallable)
	void Rebuild();

};