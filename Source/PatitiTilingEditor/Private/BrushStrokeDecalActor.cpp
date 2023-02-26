// Fill out your copyright notice in the Description page of Project Settings.


#include "BrushStrokeDecalActor.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/DecalComponent.h"

void ABrushStrokeDecalActor::CreateDynamicMaterial()
{
	UMaterialInterface* MaterialParent = this->GetDecalMaterial();
	if(!MaterialParent) return;
	UMaterialInstanceDynamic* DynamicMaterial =UKismetMaterialLibrary::CreateDynamicMaterialInstance(GWorld, MaterialParent);
	this->SetDecalMaterial(DynamicMaterial);
}


