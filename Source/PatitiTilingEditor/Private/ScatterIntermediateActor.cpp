#include "ScatterIntermediateActor.h"
#include "Engine/StaticMeshActor.h"
#include "GeometryScript/SceneUtilityFunctions.h"
#include "GeometryScript/MeshTransformFunctions.h"
#include "GeometryScript/MeshSpatialFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "BrushStrokeDecalActor.h"
#include "Components/DecalComponent.h"

AScatterIntermediateActor::AScatterIntermediateActor(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void AScatterIntermediateActor::GenerateBrushes()
{
	for(auto Actor : GeneratedActors)
	{
		Actor->Destroy();
	}
	GeneratedActors.Empty();

	if (!BrushesTransform.Num() || !BrushesSize.Num() || !InStaticMeshActor || !BrushesMaterial.Num()) return;
	FRandomStream RandomStream = UKismetMathLibrary::MakeRandomStream(RandomSeed);
	for (int Index = 0; Index < BrushesTransform.Num(); Index++)
	{
		FVector Location = BrushesTransform[Index].GetLocation();
		FRotator Rotation = BrushesTransform[Index].GetRotation().Rotator();
		Location += InStaticMeshActor->GetActorLocation();
		ABrushStrokeDecalActor* SpawnedDecalActor = GWorld->SpawnActor<ABrushStrokeDecalActor>(Location, Rotation);
		GeneratedActors.Add(SpawnedDecalActor);
		int BrushMaterialNum = UKismetMathLibrary::RandomIntegerInRangeFromStream(0, BrushesMaterial.Num() - 1, RandomStream);
		UMaterialInterface* Material = BrushesMaterial[BrushMaterialNum];

		SpawnedDecalActor->GetDecal()->DecalSize = FVector(1, BrushesSize[Index], BrushesSize[Index]);
		SpawnedDecalActor->GetDecal()->SetMaterial(0, Material);
		SpawnedDecalActor->CreateDynamicMaterial();
		UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(SpawnedDecalActor->GetDecal()->GetMaterial(0));
		if (!DynamicMaterial) continue;
		FVector4 RandomColor;
		FLinearColor HSVMin = ColorRangeMin.LinearRGBToHSV();
		FLinearColor HSVMax = ColorRangeMax.LinearRGBToHSV();

		FLinearColor HSV;

		HSV = FLinearColor(
			HSVMin.R < HSVMax.R ? UKismetMathLibrary::RandomFloatInRangeFromStream(HSVMin.R, HSVMax.R, RandomStream) : UKismetMathLibrary::RandomFloatInRangeFromStream(HSVMax.R, HSVMin.R, RandomStream),
			HSVMin.G < HSVMax.G ? UKismetMathLibrary::RandomFloatInRangeFromStream(HSVMin.G, HSVMax.G, RandomStream) : UKismetMathLibrary::RandomFloatInRangeFromStream(HSVMax.G, HSVMin.G, RandomStream),
			HSVMin.B < HSVMax.B ? UKismetMathLibrary::RandomFloatInRangeFromStream(HSVMin.B, HSVMax.B, RandomStream) : UKismetMathLibrary::RandomFloatInRangeFromStream(HSVMax.B, HSVMin.B, RandomStream),
			1);
		HSV = HSV.HSVToLinearRGB();
		RandomColor[0] = HSV.R;
		RandomColor[1] = HSV.G;
		RandomColor[2] = HSV.B;
		RandomColor[3] = 1;
		DynamicMaterial->SetVectorParameterValue("BrushStrokeColor", RandomColor);
	}
}

void AScatterIntermediateActor::Rebuild()
{
	UDynamicMesh* TargetMesh = this->GetDynamicMeshComponent()->GetDynamicMesh();
	TargetMesh->Reset();
	this->OnRebuildGeneratedMesh(TargetMesh);
	if (!InStaticMeshActor) return;
	BrushesTransform.Empty();
	BrushesSize.Empty();

	FTransform LocalToWorld;
	TEnumAsByte<EGeometryScriptOutcomePins> OutCome;
	UGeometryScriptLibrary_SceneUtilityFunctions::CopyMeshFromComponent(InStaticMeshActor->GetStaticMeshComponent(), TargetMesh, FGeometryScriptCopyMeshFromComponentOptions(), false, LocalToWorld, OutCome, nullptr);
	FVector Scale = InStaticMeshActor->GetActorScale3D();
	UGeometryScriptLibrary_MeshTransformFunctions::ScaleMesh(TargetMesh, Scale);

	FRandomStream RandomStream = UKismetMathLibrary::MakeRandomStream(RandomSeed);
	FGeometryScriptDynamicMeshBVH MeshBVH;
	UGeometryScriptLibrary_MeshSpatial::BuildBVHForMesh(TargetMesh, MeshBVH);
	FBox BoundingBox = UGeometryScriptLibrary_MeshQueryFunctions::GetMeshBoundingBox(TargetMesh);
	FVector Center = (BoundingBox.Min + BoundingBox.Max) * 0.5;
	FVector Extent = (BoundingBox.Max - BoundingBox.Min) * 0.5;

	UDynamicMesh* AccumulatedMesh = this->AllocateComputeMesh();
	for (int Index = 0; Index < BrushNumber; Index++)
	{
		FVector QueryPoint = UKismetMathLibrary::RandomPointInBoundingBoxFromStream(Center, Extent, RandomStream);
		FGeometryScriptTrianglePoint NearestPoint;
		TEnumAsByte<EGeometryScriptSearchOutcomePins> InOutCome;
		UGeometryScriptLibrary_MeshSpatial::FindNearestPointOnMesh(TargetMesh, MeshBVH, QueryPoint, FGeometryScriptSpatialQueryOptions(), NearestPoint, InOutCome);
		bool IsValidTriangle;
		FVector Normal = UGeometryScriptLibrary_MeshQueryFunctions::GetTriangleFaceNormal(TargetMesh, NearestPoint.TriangleID, IsValidTriangle);
		if(Normal.Z < 0) continue;
		if (!IsValidTriangle) return;
		FRotator Rotation = UKismetMathLibrary::MakeRotFromZ(Normal);
		FTransform Transform = FTransform(Rotation, NearestPoint.Position);
		float Size = UKismetMathLibrary::RandomFloatInRangeFromStream(DecalSizeMin, DecalSizeMax, RandomSeed);
		UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(AccumulatedMesh, FGeometryScriptPrimitiveOptions(), Transform, Size, Size, 50);

		BrushesTransform.Add(Transform);
		BrushesSize.Add(Size);
	}
	UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(TargetMesh, AccumulatedMesh, FTransform());
	this->ReleaseAllComputeMeshes();
}

