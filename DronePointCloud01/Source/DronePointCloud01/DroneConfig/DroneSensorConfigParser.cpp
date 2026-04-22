#include "DroneSensorConfigParser.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"

bool FDroneSensorConfigParser::LoadFromJsonString(const FString& JsonString, FDroneSensorConfig& OutConfig, FString* OutError)
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    TSharedPtr<FJsonObject> RootObject;
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        if (OutError)
        {
            *OutError = TEXT("JSON deserialize failed.");
        }
        return false;
    }

    return ParseRoot(RootObject, OutConfig, OutError);
}

bool FDroneSensorConfigParser::LoadFromJsonFile(const FString& FilePath, FDroneSensorConfig& OutConfig, FString* OutError)
{
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        if (OutError)
        {
            *OutError = FString::Printf(TEXT("Failed to load file: %s"), *FilePath);
        }
        return false;
    }

    return LoadFromJsonString(JsonString, OutConfig, OutError);
}

bool FDroneSensorConfigParser::ParseRoot(const TSharedPtr<FJsonObject>& RootObject, FDroneSensorConfig& OutConfig, FString* OutError)
{
    const TSharedPtr<FJsonObject>* LidarObject = nullptr;
    if (!RootObject->TryGetObjectField(TEXT("lidar"), LidarObject) || !LidarObject || !LidarObject->IsValid())
    {
        if (OutError)
        {
            *OutError = TEXT("Missing or invalid 'lidar' object.");
        }
        return false;
    }

    const TSharedPtr<FJsonObject>* CameraObject = nullptr;
    if (!RootObject->TryGetObjectField(TEXT("camera"), CameraObject) || !CameraObject || !CameraObject->IsValid())
    {
        if (OutError)
        {
            *OutError = TEXT("Missing or invalid 'camera' object.");
        }
        return false;
    }

    if (!ParseLidar(*LidarObject, OutConfig.Lidar, OutError))
    {
        return false;
    }

    if (!ParseCamera(*CameraObject, OutConfig.Camera, OutError))
    {
        return false;
    }

    return true;
}

bool FDroneSensorConfigParser::ParseLidar(const TSharedPtr<FJsonObject>& LidarObject, FLidarConfig& OutLidar, FString* OutError)
{
    if (!GetStringField(LidarObject, TEXT("type"), OutLidar.Type, OutError)) return false;
    if (!GetIntField(LidarObject, TEXT("channels"), OutLidar.Channels, OutError)) return false;
    if (!GetNumberField(LidarObject, TEXT("rpm"), OutLidar.Rpm, OutError)) return false;
    if (!GetNumberField(LidarObject, TEXT("h_fov_deg"), OutLidar.HFovDeg, OutError)) return false;
    if (!GetNumberField(LidarObject, TEXT("v_fov_deg"), OutLidar.VFovDeg, OutError)) return false;
    if (!GetNumberField(LidarObject, TEXT("range_noise_std_m"), OutLidar.RangeNoiseStdM, OutError)) return false;
    if (!GetNumberField(LidarObject, TEXT("dropout_rate"), OutLidar.DropoutRate, OutError)) return false;
    if (!GetBoolField(LidarObject, TEXT("intensity"), OutLidar.bIntensity, OutError)) return false;

    const TSharedPtr<FJsonObject>* RangeObject = nullptr;
    if (!LidarObject->TryGetObjectField(TEXT("range_m"), RangeObject) || !RangeObject || !RangeObject->IsValid())
    {
        if (OutError)
        {
            *OutError = TEXT("Missing or invalid 'lidar.range_m' object.");
        }
        return false;
    }

    if (!GetNumberField(*RangeObject, TEXT("min"), OutLidar.RangeM.Min, OutError)) return false;
    if (!GetNumberField(*RangeObject, TEXT("max"), OutLidar.RangeM.Max, OutError)) return false;

    const TSharedPtr<FJsonObject>* PoseObject = nullptr;
    if (!LidarObject->TryGetObjectField(TEXT("pose"), PoseObject) || !PoseObject || !PoseObject->IsValid())
    {
        if (OutError)
        {
            *OutError = TEXT("Missing or invalid 'lidar.pose' object.");
        }
        return false;
    }

    if (!GetNumberField(*PoseObject, TEXT("x"), OutLidar.Pose.X, OutError)) return false;
    if (!GetNumberField(*PoseObject, TEXT("y"), OutLidar.Pose.Y, OutError)) return false;
    if (!GetNumberField(*PoseObject, TEXT("z"), OutLidar.Pose.Z, OutError)) return false;

    return true;
}

bool FDroneSensorConfigParser::ParseCamera(const TSharedPtr<FJsonObject>& CameraObject, FCameraConfig& OutCamera, FString* OutError)
{
    if (!GetNumberField(CameraObject, TEXT("fov_deg"), OutCamera.FovDeg, OutError)) return false;
    if (!GetIntField(CameraObject, TEXT("fps"), OutCamera.Fps, OutError)) return false;
    if (!GetStringField(CameraObject, TEXT("distortion"), OutCamera.Distortion, OutError)) return false;

    const TArray<TSharedPtr<FJsonValue>>* ResolutionArray = nullptr;
    if (!CameraObject->TryGetArrayField(TEXT("resolution"), ResolutionArray) || !ResolutionArray || ResolutionArray->Num() != 2)
    {
        if (OutError)
        {
            *OutError = TEXT("Missing or invalid 'camera.resolution' array.");
        }
        return false;
    }

    OutCamera.Resolution.X = static_cast<int32>((*ResolutionArray)[0]->AsNumber());
    OutCamera.Resolution.Y = static_cast<int32>((*ResolutionArray)[1]->AsNumber());

    return true;
}

bool FDroneSensorConfigParser::GetNumberField(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, float& OutValue, FString* OutError)
{
    double TempValue = 0.0;
    if (!Obj->TryGetNumberField(FieldName, TempValue))
    {
        if (OutError)
        {
            *OutError = FString::Printf(TEXT("Missing or invalid number field: %s"), *FieldName);
        }
        return false;
    }

    OutValue = static_cast<float>(TempValue);
    return true;
}

bool FDroneSensorConfigParser::GetIntField(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, int32& OutValue, FString* OutError)
{
    double TempValue = 0.0;
    if (!Obj->TryGetNumberField(FieldName, TempValue))
    {
        if (OutError)
        {
            *OutError = FString::Printf(TEXT("Missing or invalid int field: %s"), *FieldName);
        }
        return false;
    }

    OutValue = static_cast<int32>(TempValue);
    return true;
}

bool FDroneSensorConfigParser::GetBoolField(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, bool& OutValue, FString* OutError)
{
    if (!Obj->TryGetBoolField(FieldName, OutValue))
    {
        if (OutError)
        {
            *OutError = FString::Printf(TEXT("Missing or invalid bool field: %s"), *FieldName);
        }
        return false;
    }

    return true;
}

bool FDroneSensorConfigParser::GetStringField(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, FString& OutValue, FString* OutError)
{
    if (!Obj->TryGetStringField(FieldName, OutValue))
    {
        if (OutError)
        {
            *OutError = FString::Printf(TEXT("Missing or invalid string field: %s"), *FieldName);
        }
        return false;
    }

    return true;
}