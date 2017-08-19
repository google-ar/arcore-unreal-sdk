// Copyright 2017 Google Inc.
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
#include "GameFramework/Actor.h"

#include "ARPlaneActor.generated.h"

UCLASS()
class HELLOARUNREAL_API AARPlaneActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AARPlaneActor();

	/* The procedural mesh component */
	UPROPERTY(Category = GoogleARCorePlaneActor, EditAnywhere, BlueprintReadWrite)
	class UProceduralMeshComponent* PlanePolygonMeshComponent;

	/** When set to true, the actor will remove the ARAnchor object from the current tracking session when the Actor gets destroyed.*/
	UPROPERTY(Category = GoogleARCorePlaneActor, BlueprintReadWrite)
	class UGoogleARCorePlane* ARCorePlaneObject = nullptr;

	/** The feathering distance for the polygon edge. Default to 10 cm*/
	UPROPERTY(Category = GoogleARCorePlaneActor, EditAnywhere, BlueprintReadWrite)
	float EdgeFeatheringDistance = 10.0f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void UpdatePlanePolygonMesh();

};
