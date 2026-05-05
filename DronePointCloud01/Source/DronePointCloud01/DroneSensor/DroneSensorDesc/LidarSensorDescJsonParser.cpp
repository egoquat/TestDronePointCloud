#include "LidarSensorDescJsonParser.h"

#include "Dom/JsonObject.h"
#include "DroneSensor/SensorDescription.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FLidarSensorDescJsonParser::LoadFromJsonFile(
	const FString& FilePath,
	FLidarSensorDescription& OutDescription)
{
	TSharedPtr<FJsonObject> JsonObject;

	if (!ReadJsonObject(FilePath, JsonObject))
	{
		UE_LOG(LogTemp, Error, TEXT(">> LoadLidarSensorDescription, Failed, %s"), *FilePath);
		return false;
	}

	TryGetUInt(JsonObject, TEXT("Channels"), OutDescription.Channels);
	TryGetFloatArray(JsonObject, TEXT("ChannelAngles"), OutDescription.ChannelAngles);

	TryGetUInt(JsonObject, TEXT("ResolutionHorizontal"), OutDescription.ResolutionHorizontal);
	TryGetFloat(JsonObject, TEXT("RangeMin"), OutDescription.RangeMin);
	TryGetFloat(JsonObject, TEXT("RangeMax"), OutDescription.RangeMax);
	TryGetFloat(JsonObject, TEXT("RotationFrequencyHZ"), OutDescription.RotationFrequencyHZ);
	TryGetFloat(JsonObject, TEXT("UpperFovLimit"), OutDescription.UpperFovLimit);
	TryGetFloat(JsonObject, TEXT("LowerFovLimit"), OutDescription.LowerFovLimit);
	TryGetFloat(JsonObject, TEXT("HorizontalFov"), OutDescription.HorizontalFov);
	TryGetInt(JsonObject, TEXT("RandomSeed"), OutDescription.RandomSeed);

	TryGetBool(JsonObject, TEXT("bUseLidarNoise"), OutDescription.bUseLidarNoise);
	TryGetFloat(JsonObject, TEXT("NoiseStdDev"), OutDescription.NoiseStdDev);
	TryGetDouble(JsonObject, TEXT("NoiseCo2Curve"), OutDescription.NoiseCo2Curve);
	TryGetDouble(JsonObject, TEXT("NoiseCo1Curve"), OutDescription.NoiseCo1Curve);
	TryGetDouble(JsonObject, TEXT("NoiseCo0Curve"), OutDescription.NoiseCo0Curve);
	TryGetFloat(JsonObject, TEXT("NoiseMeanRangeError"), OutDescription.NoiseMeanRangeError);

	TryGetFloat(JsonObject, TEXT("SizeBaseDots"), OutDescription.SizeBaseDots);
	TryGetFloat(JsonObject, TEXT("AtmospAttenRate"), OutDescription.AtmospAttenRate);
	TryGetFloat(JsonObject, TEXT("DropOffGenRate"), OutDescription.DropOffGenRate);
	TryGetFloat(JsonObject, TEXT("DropOffIntensityLimit"), OutDescription.DropOffIntensityLimit);
	TryGetFloat(JsonObject, TEXT("DropOffAtZeroIntensity"), OutDescription.DropOffAtZeroIntensity);

	TryGetBool(JsonObject, TEXT("bVisualizeIntensity"), OutDescription.bVisualizeIntensity);
	TryGetFloat(JsonObject, TEXT("RatioIntensityAtmos"), OutDescription.RatioIntensityAtmos);
	TryGetFloat(JsonObject, TEXT("RatioIntensityCosin"), OutDescription.RatioIntensityCosin);
	TryGetFloat(JsonObject, TEXT("RatioIntensityRefle"), OutDescription.RatioIntensityRefle);
	TryGetFloat(JsonObject, TEXT("RatioIntensityColor"), OutDescription.RatioIntensityColor);

	UE_LOG(LogTemp, Log, TEXT(">> LoadLidarSensorDescription, Succeed, %s"), *FilePath);
	return true;
}

bool FLidarSensorDescJsonParser::ReadJsonObject(
	const FString& FilePath,
	TSharedPtr<FJsonObject>& OutJsonObject)
{
	FString JsonString;

	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return false;
	}

	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(JsonReader, OutJsonObject))
	{
		return false;
	}

	return OutJsonObject.IsValid();
}

void FLidarSensorDescJsonParser::TryGetUInt(
	const TSharedPtr<FJsonObject>& JsonObject,
	const FString& FieldName,
	uint32& OutValue)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	double Value = 0.0;

	if (JsonObject->TryGetNumberField(FieldName, Value))
	{
		OutValue = static_cast<uint32>(FMath::Max(0.0, Value));
	}
}

void FLidarSensorDescJsonParser::TryGetInt(
	const TSharedPtr<FJsonObject>& JsonObject,
	const FString& FieldName,
	int32& OutValue)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	int32 Value = 0;

	if (JsonObject->TryGetNumberField(FieldName, Value))
	{
		OutValue = Value;
	}
}

void FLidarSensorDescJsonParser::TryGetFloat(
	const TSharedPtr<FJsonObject>& JsonObject,
	const FString& FieldName,
	float& OutValue)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	double Value = 0.0;

	if (JsonObject->TryGetNumberField(FieldName, Value))
	{
		OutValue = static_cast<float>(Value);
	}
}

void FLidarSensorDescJsonParser::TryGetDouble(
	const TSharedPtr<FJsonObject>& JsonObject,
	const FString& FieldName,
	double& OutValue)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	double Value = 0.0;

	if (JsonObject->TryGetNumberField(FieldName, Value))
	{
		OutValue = Value;
	}
}

void FLidarSensorDescJsonParser::TryGetBool(
	const TSharedPtr<FJsonObject>& JsonObject,
	const FString& FieldName,
	bool& OutValue)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	bool Value = false;

	if (JsonObject->TryGetBoolField(FieldName, Value))
	{
		OutValue = Value;
	}
}

void FLidarSensorDescJsonParser::TryGetFloatArray(
	const TSharedPtr<FJsonObject>& JsonObject,
	const FString& FieldName,
	TArray<float>& OutValue)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* JsonArray = nullptr;

	if (!JsonObject->TryGetArrayField(FieldName, JsonArray))
	{
		return;
	}

	OutValue.Reset();

	for (const TSharedPtr<FJsonValue>& JsonValue : *JsonArray)
	{
		if (!JsonValue.IsValid())
		{
			continue;
		}

		OutValue.Add(static_cast<float>(JsonValue->AsNumber()));
	}
}