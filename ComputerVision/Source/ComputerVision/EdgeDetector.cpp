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

#include "EdgeDetector.h"

#include "ComputerVision.h"

#include "GoogleARCoreCameraImage.h"
#include "GoogleARCoreFunctionLibrary.h"

#include "TransformCalculus2D.h"

static const int SobelThreshold = 128 * 128;
void AGoogleARCoreEdgeDetector::GoogleARCoreDoSobelEdgeDetection(
	const uint8 *InYPlaneData,
	uint32 YPlanePixelStride,
	uint32 YPlaneRowStride,
	uint8 *OutPixels,
	int32 Width,
	int32 Height)
{
	// We copy the image data here before running the edge detection algorithm.
	// This is due to on some device(Exynos S8), accessing the original image
	// buffer is extremely slow.
	TArray<uint8> YPlaneDataCopy(InYPlaneData, YPlaneRowStride * Height);

	int XKernel[3][3] = {
		{ -1, 0, 1 },
		{ -2, 0, 2 },
		{ -1, 0, 1 }
	};

	int YKernel[3][3] = {
		{ -1, -2, -1 },
		{ 0,  0,  0 },
		{ 1,  2,  1 }
	};

	for (int32 y = 0; y < Height; y++)
	{
		for (int32 x = 0; x < Width; x++)
		{
			int XMag = 0;
			int YMag = 0;

			for (int32 u = 0; u < 3; u++)
			{
				for (int32 v = 0; v < 3; v++)
				{
					int32 u2 = x + u - 1;
					int32 v2 = y + v - 1;

					if (u2 < 0) u2 = 0;
					if (u2 >= Width) u2 = Width - 1;
					if (v2 < 0) v2 = 0;
					if (v2 >= Height) v2 = Height - 1;

					uint8 SourcePixel = YPlaneDataCopy[
						u2 * YPlanePixelStride +
							v2 * YPlaneRowStride];

					XMag += SourcePixel * XKernel[u][v];
					YMag += SourcePixel * YKernel[u][v];
				}
			}

			int Magnitude = XMag * XMag + YMag * YMag;
			uint8 Output = Magnitude > SobelThreshold ? 0xFF : 0x1F;
			OutPixels[y * Width + x] = Output;
		}
	}
}

EGoogleARCoreFunctionStatus AGoogleARCoreEdgeDetector::UpdateCameraImage()
{
	EGoogleARCoreFunctionStatus AcquireStatus =
		EGoogleARCoreFunctionStatus::NotAvailable;

#if PLATFORM_ANDROID

	UGoogleARCoreCameraImage *CameraImage = nullptr;
	AcquireStatus =
		UGoogleARCoreFrameFunctionLibrary::AcquireCameraImage(CameraImage);
	if(AcquireStatus != EGoogleARCoreFunctionStatus::Success)
	{
		return AcquireStatus;
	}

	int32_t Width = CameraImage->GetWidth();
	int32_t Height = CameraImage->GetHeight();
	int32_t planeCount = CameraImage->GetPlaneCount();

	if (!CameraImageTexture || CameraImageTexture->GetSizeX() != Width || CameraImageTexture->GetSizeY() != Height)
	{
		CameraImageTexture = UTexture2D::CreateTransient(Width, Height, EPixelFormat::PF_G8);
		CameraImageTexture->UpdateResource();
	}

	uint8_t *TempRGBABuf = new uint8_t[Width * Height];

	// Y
	int32_t y_xStride = 0;
	int32_t y_yStride = 0;
	int32_t y_length = 0;
	uint8_t *y_planeData = nullptr;
	y_planeData = CameraImage->GetPlaneData(0, y_xStride, y_yStride, y_length);

	// U
	int32_t u_xStride = 0;
	int32_t u_yStride = 0;
	int32_t u_length = 0;
	uint8_t *u_planeData = nullptr;
	u_planeData = CameraImage->GetPlaneData(1, u_xStride, u_yStride, u_length);

	// V
	int32_t v_xStride = 0;
	int32_t v_yStride = 0;
	int32_t v_length = 0;
	uint8_t *v_planeData = nullptr;
	v_planeData = CameraImage->GetPlaneData(2, v_xStride, v_yStride, v_length);

	GoogleARCoreDoSobelEdgeDetection(
		y_planeData, y_xStride, y_yStride, TempRGBABuf, Width, Height);

	FUpdateTextureRegion2D *Region = new FUpdateTextureRegion2D[1];
	Region->DestX = 0;
	Region->DestY = 0;
	Region->SrcX = 0;
	Region->SrcY = 0;
	Region->Width = Width;
	Region->Height = Height;

	auto CleanupData = [](uint8_t *SrcData, const FUpdateTextureRegion2D* Regions)
		{
			delete[] SrcData;
			delete[] Regions;
		};

	CameraImageTexture->UpdateTextureRegions(
		0, 1, Region, Width, 1,
		reinterpret_cast<uint8_t*>(TempRGBABuf),
		CleanupData);

	CameraImage->Release();

#endif

	return AcquireStatus;
}

UTexture2D *AGoogleARCoreEdgeDetector::GetCameraImage()
{
	return CameraImageTexture;
}

void AGoogleARCoreEdgeDetector::GetTransformedCameraImageUV(TArray<float>& OutUV)
{
	if(!CameraImageTexture) {
		return;
	}

	// First get the cropped UV so that the camera image and pass through camera image has the same aspect ratio.
	FIntPoint ScreenSize = GEngine->GameViewport->Viewport->GetSizeXY();
	int32 TexWidth = CameraImageTexture->GetSizeX();
	int32 TexHeight = CameraImageTexture->GetSizeY();
	float TexAspect = static_cast<float>(TexWidth) / static_cast<float>(TexHeight);
	// Hard coded the passthrough image aspect ratio as it is hard coded in the ARCore Plugin.
	float PassthroughAspect = 1920.0f / 1080.0f; 

	bool bLandscape = true;
	if (ScreenSize.X < ScreenSize.Y)
	{
		bLandscape = false;
	}

	float MinU = 0.0f;
	float MinV = 0.0f;

	if(TexAspect < PassthroughAspect) {
		// Texture needs to be cropped on top/bottom.
		MinV = (1 - TexAspect / PassthroughAspect) / 2.0f;
	} else {
		// Texture needs to be cropped on the left/right.
		MinU = (1 - PassthroughAspect / TexAspect) / 2.0f;
	}
	
	if (!bLandscape)
	{
		float Temp = MinU;
		MinU = MinV;
		MinV = Temp;
	}

	const TArray<float> CroppedUV = { MinU, MinV, 1.0f - MinU, MinV, MinU, 1.0f - MinV, 1.0f - MinU, 1.0f - MinV };

	// Transform the cropped UV.
	UGoogleARCoreSessionFunctionLibrary::GetPassthroughCameraImageUV(CroppedUV, OutUV);
}
