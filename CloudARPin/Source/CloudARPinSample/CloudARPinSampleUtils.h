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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FindSessionsCallbackProxy.h"

#include "CloudARPinSampleUtils.generated.h"

UCLASS()
class CLOUDARPINSAMPLE_API UCloudARPinSampleUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UCloudARPinSampleUtils")
	static FString GetLocalHostIPAddress();

	UFUNCTION(BlueprintPure, Category = "UCloudARPinSampleUtils")
	static FString GetSessionId(const FBlueprintSessionResult& Result);

	UFUNCTION(BlueprintPure, Category = "UCloudARPinSampleUtils", meta = (WorldContext = "WorldContextObject"))
	static FString GetHostSessionId(UObject* WorldContextObject);
};
