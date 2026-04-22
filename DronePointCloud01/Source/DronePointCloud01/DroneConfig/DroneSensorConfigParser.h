#pragma once

#include "CoreMinimal.h"
#include "DroneSensorConfig.h"

class FDroneSensorConfigParser
{
public:
	static bool LoadFromJsonString(const FString& JsonString, FDroneSensorConfig& OutConfig, FString* OutError = nullptr);
	static bool LoadFromJsonFile(const FString& FilePath, FDroneSensorConfig& OutConfig, FString* OutError = nullptr);

private:
	static bool ParseRoot(const TSharedPtr<FJsonObject>& RootObject, FDroneSensorConfig& OutConfig, FString* OutError);
	static bool ParseLidar(const TSharedPtr<FJsonObject>& LidarObject, FLidarConfig& OutLidar, FString* OutError);
	static bool ParseCamera(const TSharedPtr<FJsonObject>& CameraObject, FCameraConfig& OutCamera, FString* OutError);

	static bool GetNumberField(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, float& OutValue, FString* OutError);
	static bool GetIntField(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, int32& OutValue, FString* OutError);
	static bool GetBoolField(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, bool& OutValue, FString* OutError);
	static bool GetStringField(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, FString& OutValue, FString* OutError);
};