#pragma once
#include "SensorDescription.generated.h"

USTRUCT()
struct FSensorDescription
{
    GENERATED_BODY()

    UPROPERTY()
    FString NameSensor = TEXT("Default Sensor");
    
    UPROPERTY()
    FTransform TMRelativeSensor;
};

USTRUCT()
struct FLidarSensorDescription : public FSensorDescription
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, meta=(ClampMin = "32", ClampMax = "128", UIMin = "32", UIMax = "128"))
    uint32 Channels = 32;
	
	UPROPERTY(EditAnywhere)
	TArray<float> ChannelAngles;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin = "128", ClampMax = "2048", UIMin = "128", UIMax = "2048"))
    uint32 ResolutionHorizontal = 512;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "800", UIMin = "0", UIMax = "800"))
    float RangeMin = 80.0f;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin = "1000", ClampMax = "35000", UIMin = "1000", UIMax = "35000"))
    float RangeMax = 10000.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "2", ClampMax = "12", UIMin = "2", UIMax = "12"))
    float RotationFrequencyHZ = 8;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "20", UIMin = "0", UIMax = "20"))
    float UpperFovLimit = 20.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "-45", ClampMax = "-10", UIMin = "-45", UIMax = "-10"))
    float LowerFovLimit = -20.0f;
  
    UPROPERTY(EditAnywhere, meta=(ClampMin = "90", ClampMax = "360", UIMin = "90", UIMax = "360"))
    float HorizontalFov = 360.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "999", UIMin = "0", UIMax = "999"))
    int RandomSeed = 0;
    
	UPROPERTY(EditAnywhere)
	bool bUseLidarNoise = true;
	
    UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "0.2", UIMin = "0", UIMax = "0.2"))
    float NoiseStdDev = 0.1f;
    
	//이하 참고: https://data.ouster.io/downloads/datasheets/datasheet-rev7-v2p5-os2.pdf
	UPROPERTY(EditAnywhere, meta=(ClampMin = "-2.0", ClampMax = "2.0", UIMin = "-2.0", UIMax = "2.0"))
	double NoiseCo2Curve = 0.00023;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "-2.0", ClampMax = "2.0", UIMin = "-2.0", UIMax = "2.0"))
	double NoiseCo1Curve = -0.005702;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "-5.0", ClampMax = "5.0", UIMin = "-5.0", UIMax = "5.0"))
	double NoiseCo0Curve = 1.8;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "10.0", UIMin = "0.0", UIMax = "10.0"))
	float NoiseMeanRangeError = 0.0f;
	
    UPROPERTY(EditAnywhere, meta=(ClampMin = "0.01", ClampMax = "10.0", UIMin = "0.01", UIMax = "10.0"))
    float SizeBaseDot = 0.7f;
	
	/// Attenuation Rate in the atmosphere in m^-1.
	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.003", ClampMax = "0.1", UIMin = "0.003", UIMax = "0.1"))
	float AtmospAttenRate = 0.004f;
	
	/// General drop off rate.
	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.2", ClampMax = "0.6", UIMin = "0.2", UIMax = "0.6"))
	float DropOffGenRate = 0.45f;

	/// General drop off rate.
	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.6", ClampMax = "1.0", UIMin = "0.6", UIMax = "1.0"))
	float DropOffIntensityLimit = 0.8f;

	/// General drop off rate.
	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.2", ClampMax = "0.6", UIMin = "0.2", UIMax = "0.6"))
	float DropOffAtZeroIntensity = 0.4f;

	UPROPERTY(EditAnywhere)
	bool bVisualizeIntensity = true;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "1.0", UIMin = "0", UIMax = "1.0"))
	float RatioIntensityAtmos = 1.0f;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "1.0", UIMin = "0", UIMax = "1.0"))
	float RatioIntensityCosin = 0.7f;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "1.0", UIMin = "0", UIMax = "1.0"))
	float RatioIntensityRefle = 1.0f;
	
	UPROPERTY(EditAnywhere, meta=(ClampMin = "0", ClampMax = "1.0", UIMin = "0", UIMax = "1.0"))
	float RatioIntensityColor = 0.2f;
};

USTRUCT()
struct FCameraSensorDescription : public FSensorDescription
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float TargetGamma = 2.2f;

	UPROPERTY(EditAnywhere)
	uint32 ImageWidth = 1024;

	UPROPERTY(EditAnywhere)
	uint32 ImageHeight = 1024;

	UPROPERTY(EditAnywhere)
	bool bEnable16BitFormat = false;

	UPROPERTY(EditAnywhere, Category=Projection, meta=(DisplayName = "Field of View", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0"))
	float FOVAngle = 90;
	
	UPROPERTY(EditAnywhere, Category="Lens|Lens Flares", meta=(UIMin = "0.0", UIMax = "16.0", editcondition = "LensFlareIntensity"))
	float LensFlareIntensity = 0.1f;
	
	UPROPERTY(EditAnywhere, Category="Lens|Bloom", meta=(ClampMin = "0.0", UIMax = "8.0", editcondition = "BloomIntensity"))
	float BloomIntensity = 0.675;

	UPROPERTY(EditAnywhere, Category="Lens|Camera", meta=(ClampMin = "1.0", ClampMax = "2000.0", editcondition = "CameraShutterSpeed", DisplayName = "Shutter Speed (1/s)"))
    float CameraShutterSpeed = 200.0f;
	
	UPROPERTY(EditAnywhere, Category="Lens|Camera", meta=(ClampMin = "1.0", tooltip = "The camera sensor sensitivity", editcondition = "CameraISO", DisplayName = "ISO"))
    float CameraISO = 100.0f;
	
	UPROPERTY(EditAnywhere, Category="Lens|Camera", meta=(ClampMin = "1.0", ClampMax = "32.0", editcondition = "DepthOfFieldFstop", DisplayName = "Aperture (F-stop)"))
	float DepthOfFieldFstop = 1.4f;

    /**
     * Logarithmic adjustment for the exposure. Only used if a tonemapper is specified.
     * 0: no adjustment, -1:2x darker, -2:4x darker, 1:2x brighter, 2:4x brighter, ...
     */
    UPROPERTY(EditAnywhere, Category = "Lens|Exposure", meta = (UIMin = "-15.0", UIMax = "15.0", DisplayName = "Exposure Compensation "))
    float AutoExposureBiasCompensation = 0.0f;
};