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

#include "ComputerVision.h"

#include "EdgeDetector.generated.h"

/**
 * This class demonstrates how to access ARCore camera image data on
 * the CPU.
 */
UCLASS(Blueprintable, BlueprintType)
class AGoogleARCoreEdgeDetector : public AActor
{
	GENERATED_BODY()

public:

	/**
	 * This function acquires a new CPU-accessible camera image, runs
	 * a sobel edge detection filter on it on the CPU, and generates a
	 * texture so that it can be displayed as a demonstration of CPU
	 * image access.
	 */
	UFUNCTION(BlueprintCallable, Category = "GoogleARCoreSample|EdgeDetector", meta = (Keywords = "googlear arcore edgedetector"))
	EGoogleARCoreFunctionStatus UpdateCameraImage();

	/**
	 * This function gets the texture generated with
	 * UpdateCameraImage().
	 *
	 * @return The generated texture.
	 */
	UFUNCTION(BlueprintCallable, Category = "GoogleARCoreSample|EdgeDetector", meta = (Keywords = "googlear arcore edgedetector"))
	UTexture2D *GetCameraImage();

	/**
	 * Generate the texture coordinates that matches the camera image with the passthrough image aspect ratio and rotation.
	 */
	UFUNCTION(BlueprintPure, Category = "GoogleARCoreSample|EdgeDetector", meta = (Keywords = "googlear arcore edgedetector"))
	void GetTransformedCameraImageUV(TArray<float>& OutUV);

	/**
	 * The generated camera texture.
	 */
	UPROPERTY()
	UTexture2D *CameraImageTexture = nullptr;

private:

	static void GoogleARCoreDoSobelEdgeDetection(
		const uint8 *InYPlaneData,
		uint32 YPlanePixelStride,
		uint32 YPlaneRowStride,
		uint8 *OutPixels,
		int32 Width,
		int32 Height);
};

