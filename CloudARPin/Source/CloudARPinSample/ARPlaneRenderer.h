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
#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ARTrackable.h"

#include "ARPlaneRenderer.generated.h"

UCLASS()
class CLOUDARPINSAMPLE_API AARPlaneRenderer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AARPlaneRenderer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** The feathering distance for the polygon edge. Default to 10 cm*/
	UPROPERTY(Category = ARPlaneRenderer, EditAnywhere, BlueprintReadWrite)
	float EdgeFeatheringDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* PlaneMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FColor> PlaneColors;


private:
	void UpdatePlane(UARPlaneGeometry* ARCorePlaneObject);
	void UpdatePlaneMesh(UARPlaneGeometry* ARCorePlaneObject, UProceduralMeshComponent* PlanePolygonMeshComponent);

	UPROPERTY()
	TMap<UARPlaneGeometry*, UProceduralMeshComponent*> PlaneMeshMap;

	int NewPlaneIndex;
};
