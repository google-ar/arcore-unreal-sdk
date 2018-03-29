// Copyright 2018 Google Inc.

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
		uint32 *OutPixels,
		int32 Width,
		int32 Height);
};

