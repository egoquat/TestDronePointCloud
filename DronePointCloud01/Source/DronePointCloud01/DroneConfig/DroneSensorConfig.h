#pragma once

#include "CoreMinimal.h"
#include "DroneSensorConfig.generated.h"

USTRUCT(BlueprintType)
struct FRangeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Min = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Max = 0.0f;
};

USTRUCT(BlueprintType)
struct FPoseConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float X = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Y = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Z = 0.0f;
};

USTRUCT(BlueprintType)
struct FLidarConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Channels = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rpm = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HFovDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VFovDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRangeConfig RangeM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RangeNoiseStdM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DropoutRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIntensity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPoseConfig Pose;
};

USTRUCT(BlueprintType)
struct FCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FovDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Resolution = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Fps = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Distortion;
};

USTRUCT(BlueprintType)
struct FDroneSensorConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLidarConfig Lidar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraConfig Camera;
};