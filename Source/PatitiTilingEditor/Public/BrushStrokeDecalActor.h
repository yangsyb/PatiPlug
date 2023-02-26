// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DecalActor.h"
#include "BrushStrokeDecalActor.generated.h"

/**
 * 
 */
UCLASS()
class PATITITILINGEDITOR_API ABrushStrokeDecalActor : public ADecalActor
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, CallInEditor)
	void CreateDynamicMaterial();
};
