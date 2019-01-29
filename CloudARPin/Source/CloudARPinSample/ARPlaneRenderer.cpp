// Copyright 2018 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ARPlaneRenderer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ARBlueprintLibrary.h"

// Sets default values
AARPlaneRenderer::AARPlaneRenderer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	EdgeFeatheringDistance = 10.0f;
	NewPlaneIndex = 0.0f;
}

// Called when the game starts or when spawned
void AARPlaneRenderer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AARPlaneRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (UARBlueprintLibrary::GetTrackingQuality() == EARTrackingQuality::OrientationAndPosition)
	{
		TArray<UARTrackedGeometry*> AllGeometries = UARBlueprintLibrary::GetAllGeometries();
		for (UARTrackedGeometry* Geometry : AllGeometries)
		{
			if (Geometry->IsA(UARPlaneGeometry::StaticClass()))
			{
				UARPlaneGeometry* PlaneGeometry = Cast<UARPlaneGeometry>(Geometry);
				UpdatePlane(PlaneGeometry);
			}
		}
	}
}

void AARPlaneRenderer::UpdatePlane(UARPlaneGeometry* ARCorePlaneObject)
{
	UProceduralMeshComponent* PlanePolygonMeshComponent = nullptr;
	if (!PlaneMeshMap.Contains(ARCorePlaneObject))
	{
		if (ARCorePlaneObject->GetSubsumedBy() != nullptr || ARCorePlaneObject->GetTrackingState() == EARTrackingState::StoppedTracking)
		{
			return;
		}

		PlanePolygonMeshComponent = NewObject<UProceduralMeshComponent>(this);
		PlanePolygonMeshComponent->RegisterComponent();
		PlanePolygonMeshComponent->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

		UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(PlaneMaterial, this);
		FColor Color = FColor::White;
		if (PlaneColors.Num() != 0)
		{
			int ColorIndex = NewPlaneIndex % PlaneColors.Num();
			Color = PlaneColors[ColorIndex];
		}
		DynMaterial->SetScalarParameterValue(FName(TEXT("TextureRotationAngle")), FMath::FRandRange(0.0f, 1.0f));
		DynMaterial->SetVectorParameterValue(FName(TEXT("PlaneTint")), FLinearColor(Color));

		PlanePolygonMeshComponent->SetMaterial(0, DynMaterial);
		PlaneMeshMap.Add(ARCorePlaneObject, PlanePolygonMeshComponent);
		NewPlaneIndex++;
	}
	else
	{
		PlanePolygonMeshComponent = *PlaneMeshMap.Find(ARCorePlaneObject);
	}

	if(ARCorePlaneObject->GetTrackingState() == EARTrackingState::Tracking &&
	   ARCorePlaneObject->GetSubsumedBy() == nullptr)
	{
		if (!PlanePolygonMeshComponent->bVisible)
		{
			PlanePolygonMeshComponent->SetVisibility(true, true);
		}
		UpdatePlaneMesh(ARCorePlaneObject, PlanePolygonMeshComponent);
	}
	else if (PlanePolygonMeshComponent->bVisible)
	{
		PlanePolygonMeshComponent->SetVisibility(false, true);
	}
	
	if(ARCorePlaneObject->GetSubsumedBy() != nullptr || ARCorePlaneObject->GetTrackingState() == EARTrackingState::StoppedTracking)
	{
		PlanePolygonMeshComponent = *PlaneMeshMap.Find(ARCorePlaneObject);
		if(PlanePolygonMeshComponent != nullptr)
		{
			PlanePolygonMeshComponent->DestroyComponent(true);
			PlaneMeshMap.Remove(ARCorePlaneObject);
		}
	}
}

void AARPlaneRenderer::UpdatePlaneMesh(UARPlaneGeometry* ARCorePlaneObject, UProceduralMeshComponent* PlanePolygonMeshComponent)
{
	// Update polygon mesh vertex indices, using triangle fan due to its convex.
	TArray<FVector> BoundaryVertices;
	BoundaryVertices = ARCorePlaneObject->GetBoundaryPolygonInLocalSpace();
	int BoundaryVerticesNum = BoundaryVertices.Num();

	if (BoundaryVerticesNum < 3)
	{
		PlanePolygonMeshComponent->ClearMeshSection(0);
		return;
	}

	int PolygonMeshVerticesNum = BoundaryVerticesNum * 2;
	// Triangle number is interior(n-2 for convex polygon) plus perimeter (EdgeNum * 2);
	int TriangleNum = BoundaryVerticesNum - 2 + BoundaryVerticesNum * 2;

	TArray<FVector> PolygonMeshVertices;
	TArray<FLinearColor> PolygonMeshVertexColors;
	TArray<int> PolygonMeshIndices;
	TArray<FVector> PolygonMeshNormals;
	TArray<FVector2D> PolygonMeshUVs;

	PolygonMeshVertices.Empty(PolygonMeshVerticesNum);
	PolygonMeshVertexColors.Empty(PolygonMeshVerticesNum);
	PolygonMeshIndices.Empty(TriangleNum * 3);
	PolygonMeshNormals.Empty(PolygonMeshVerticesNum);

	FVector PlaneNormal = ARCorePlaneObject->GetLocalToWorldTransform().GetRotation().GetUpVector();
	for (int i = 0; i < BoundaryVerticesNum; i++)
	{
		FVector BoundaryPoint = BoundaryVertices[i];
		float BoundaryToCenterDist = BoundaryPoint.Size();
		float FeatheringDist = FMath::Min(BoundaryToCenterDist, EdgeFeatheringDistance);
		FVector InteriorPoint = BoundaryPoint - BoundaryPoint.GetUnsafeNormal() * FeatheringDist;

		PolygonMeshVertices.Add(BoundaryPoint);
		PolygonMeshVertices.Add(InteriorPoint);

		PolygonMeshUVs.Add(FVector2D(BoundaryPoint.X, BoundaryPoint.Y));
		PolygonMeshUVs.Add(FVector2D(InteriorPoint.X, InteriorPoint.Y));

		PolygonMeshNormals.Add(PlaneNormal);
		PolygonMeshNormals.Add(PlaneNormal);

		PolygonMeshVertexColors.Add(FLinearColor(0.0f, 0.f, 0.f, 0.f));
		PolygonMeshVertexColors.Add(FLinearColor(0.0f, 0.f, 0.f, 1.f));
	}

	// Generate triangle indices

	// Perimeter triangles
	for (int i = 0; i < BoundaryVerticesNum - 1; i++)
	{
		PolygonMeshIndices.Add(i * 2);
		PolygonMeshIndices.Add(i * 2 + 2);
		PolygonMeshIndices.Add(i * 2 + 1);

		PolygonMeshIndices.Add(i * 2 + 1);
		PolygonMeshIndices.Add(i * 2 + 2);
		PolygonMeshIndices.Add(i * 2 + 3);
	}

	PolygonMeshIndices.Add((BoundaryVerticesNum - 1) * 2);
	PolygonMeshIndices.Add(0);
	PolygonMeshIndices.Add((BoundaryVerticesNum - 1) * 2 + 1);


	PolygonMeshIndices.Add((BoundaryVerticesNum - 1) * 2 + 1);
	PolygonMeshIndices.Add(0);
	PolygonMeshIndices.Add(1);

	// interior triangles
	for (int i = 3; i < PolygonMeshVerticesNum - 1; i += 2)
	{
		PolygonMeshIndices.Add(1);
		PolygonMeshIndices.Add(i);
		PolygonMeshIndices.Add(i + 2);
	}

	// No need to fill uv and tangent;
	PlanePolygonMeshComponent->CreateMeshSection_LinearColor(0, PolygonMeshVertices, PolygonMeshIndices, PolygonMeshNormals, PolygonMeshUVs, PolygonMeshVertexColors, TArray<FProcMeshTangent>(), false);

	// Set the component transform to Plane's transform.
	PlanePolygonMeshComponent->SetWorldTransform(ARCorePlaneObject->GetLocalToWorldTransform());
}
