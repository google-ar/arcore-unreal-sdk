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

#include "ARPointCloudRenderer.h"
#include "ARBlueprintLibrary.h"
#include "DrawDebugHelpers.h"

#if PLATFORM_ANDROID
#include "GoogleARCoreFunctionLibrary.h"
#endif

#if PLATFORM_IOS
#include "AppleARKitBlueprintLibrary.h"
#endif


// Sets default values
AARPointCloudRenderer::AARPointCloudRenderer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AARPointCloudRenderer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AARPointCloudRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RenderPointCloud();
}

void AARPointCloudRenderer::RenderPointCloud()
{
	UWorld* World = GetWorld();
	if (!ARSystem.IsValid())
	{
		ARSystem = StaticCastSharedPtr<FARSystemBase>(GEngine->XRSystem);
	}

	if (UARBlueprintLibrary::GetTrackingQuality() == EARTrackingQuality::OrientationAndPosition)
	{
#if PLATFORM_ANDROID
		UGoogleARCorePointCloud* LatestPointCloud = nullptr;
		EGoogleARCoreFunctionStatus Status = UGoogleARCoreFrameFunctionLibrary::GetPointCloud(LatestPointCloud);
		if (Status == EGoogleARCoreFunctionStatus::Success && LatestPointCloud != nullptr && LatestPointCloud->GetPointNum() > 0)
		{
			for (int i = 0; i < LatestPointCloud->GetPointNum(); i++)
			{
				FVector PointPosition = FVector::ZeroVector;
				float PointConfidence = 0;
				LatestPointCloud->GetPoint(i, PointPosition, PointConfidence);
				DrawDebugPoint(World, PointPosition, PointSize, PointColor, false);
			}
		}
#endif

#if PLATFORM_IOS
		@autoreleasepool
		{
			FAppleARKitFrame CurrentFrame;
			if (UAppleARKitBlueprintLibrary::GetCurrentFrame(this, CurrentFrame))
			{
				ARFrame* RawARKitFrame = reinterpret_cast<ARFrame*>(CurrentFrame.NativeFrame);
				ARPointCloud* PointCloud = RawARKitFrame.rawFeaturePoints;
				for (int i = 0; i < PointCloud.count; i++)
				{
					const vector_float3* RawPosition = PointCloud.points + i;
					FVector PointTrackingPosition = FVector(-RawPosition->z, RawPosition->x, RawPosition->y) * 100;
					FVector PointPosition = (ARSystem->GetAlignmentTransform() * ARSystem->GetTrackingToWorldTransform()).TransformPosition(PointTrackingPosition);
					DrawDebugPoint(World, PointPosition, PointSize, PointColor, false);
				}
			}
		}
#endif
	}
}

