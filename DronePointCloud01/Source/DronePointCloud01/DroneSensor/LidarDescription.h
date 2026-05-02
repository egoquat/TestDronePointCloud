#pragma once
#include "LidarDescription.generated.h"

USTRUCT()
struct FLidarDescription
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, meta=(ClampMin = "32", ClampMax = "128", UIMin = "32", UIMax = "128"))
    uint32 Channels = 64;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin = "128", ClampMax = "2048", UIMin = "128", UIMax = "2048"))
    uint32 ResolutionHorizontal = 1024;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "800", UIMin = "0", UIMax = "800"))
    float RangeMin = 80.0f;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin = "1000", ClampMax = "35000", UIMin = "1000", UIMax = "35000"))
    float RangeMax = 10000.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "2", ClampMax = "12", UIMin = "2", UIMax = "12"))
    float RotationFrequency = 10.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "20", UIMin = "0", UIMax = "20"))
    float UpperFovLimit = 10.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "-45", ClampMax = "-10", UIMin = "-45", UIMax = "-10"))
    float LowerFovLimit = -30.0f;
  
    UPROPERTY(EditAnywhere, meta=(ClampMin = "90", ClampMax = "360", UIMin = "90", UIMax = "360"))
    float HorizontalFov = 360.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "999", UIMin = "0", UIMax = "999"))
    int RandomSeed = 0;

    UPROPERTY(EditAnywhere)
    bool bVisualizeIntensity = true;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "1.0", UIMin = "0", UIMax = "1.0"))
    float RatioIntensityCosin = 0.7f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "1.0", UIMin = "0", UIMax = "1.0"))
    float RatioIntensityRefle = 1.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "1.0", UIMin = "0", UIMax = "1.0"))
    float RatioIntensityColor = 0.2f;

    UPROPERTY(EditAnywhere)
    bool bUseLidarNoise = true;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "0.2", UIMin = "0", UIMax = "0.2"))
    float NoiseStdDev = 0.1f;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin = "0.01", ClampMax = "10.0", UIMin = "0.01", UIMax = "10.0"))
    float SizeBaseDots = 1.2f;
};
