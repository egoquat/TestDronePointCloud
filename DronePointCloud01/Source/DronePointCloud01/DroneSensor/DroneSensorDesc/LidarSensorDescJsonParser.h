#pragma once

#include "CoreMinimal.h"

struct FLidarSensorDescription;

class FLidarSensorDescJsonParser
{
public:
	static bool LoadFromJsonFile(const FString& FilePath, FLidarSensorDescription& OutDescription);

private:
	static bool ReadJsonObject(const FString& FilePath, TSharedPtr<FJsonObject>& OutJsonObject);

	static void TryGetUInt(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, uint32& OutValue);
	static void TryGetInt(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, int32& OutValue);
	static void TryGetFloat(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, float& OutValue);
	static void TryGetDouble(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, double& OutValue);
	static void TryGetBool(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, bool& OutValue);
	static void TryGetFloatArray(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TArray<float>& OutValue);
};