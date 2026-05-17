#pragma once

struct FColorRatio
{
    FColor Co;
    float Ratio;
};

struct FDetection
{
    FVector DetectLocalPosition;
    FVector DetectWorldPosition;
    float DetectIntensity = 0;
    
    void AddNoiseToDetectPosition(const FVector& noiseAdd)
    {
        DetectLocalPosition += noiseAdd;
        DetectWorldPosition += noiseAdd;
    }
};

UENUM()
enum class ESensorType : int8
{
    None = -1,
    Lidar,
    Radar,
    Camera,
    IMU,
    GNSS,
    NUM
};

struct FSensorCommon
{
    const static int32 MaxInt = (1 << 31) - 1;
    
    inline static uint8_t CITYSCAPES_PALETTE_MAP[][3u] = {
        {  0u,   0u,   0u},   // unlabeled     =   0u
        // cityscape
        {128u,  64u, 128u},   // road          =   1u
        {244u,  35u, 232u},   // sidewalk      =   2u
        { 70u,  70u,  70u},   // building      =   3u
        {102u, 102u, 156u},   // wall          =   4u
        {190u, 153u, 153u},   // fence         =   5u
        {153u, 153u, 153u},   // pole          =   6u
        {250u, 170u,  30u},   // traffic light =   7u
        {220u, 220u,   0u},   // traffic sign  =   8u
        {107u, 142u,  35u},   // vegetation    =   9u
        {152u, 251u, 152u},   // terrain       =  10u
        { 70u, 130u, 180u},   // sky           =  11u
        {220u,  20u,  60u},   // pedestrian    =  12u
        {255u,   0u,   0u},   // rider         =  13u
        {  0u,   0u, 142u},   // Car           =  14u
        {  0u,   0u,  70u},   // truck         =  15u
        {  0u,  60u, 100u},   // bus           =  16u
        {  0u,  80u, 100u},   // train         =  17u
        {  0u,   0u, 230u},   // motorcycle    =  18u
        {119u,  11u,  32u},   // bicycle       =  19u
        // custom
        {110u, 190u, 160u},   // static        =  20u
        {170u, 120u,  50u},   // dynamic       =  21u
        { 55u,  90u,  80u},   // other         =  22u
        { 45u,  60u, 150u},   // water         =  23u
        {157u, 234u,  50u},   // road line     =  24u
        { 81u,   0u,  81u},   // ground        =  25u
        {150u, 100u, 100u},   // bridge        =  26u
        {230u, 150u, 140u},   // rail track    =  27u
        {180u, 165u, 180u}    // guard rail    =  28u
    };
    
    inline static TArray<FColorRatio> IntensityColors {
        FColorRatio{ FColor::Blue, 0.0f },
        FColorRatio{ FColor::Green, 0.35f },
        FColorRatio{ FColor::Red, 1.0f }
    };
    
    struct FCallback
    {
        UObject* Owner = nullptr;
        ESensorType SensorType;
        TFunction<void()> Callback = nullptr;
        FCallback(UObject* owner, ESensorType sensorType, TFunction<void()> callback)
        {
            Owner = owner; SensorType = sensorType; Callback = callback;
        }
    };

    struct FCallbackCollection
    {
        TArray<FCallback> Callbacks;

        void AddCallback(UObject* owner, ESensorType sensorType, TFunction<void()> callback)
        {
            Callbacks.Add(FCallback(owner, sensorType, callback));
        }

        void RemoveAll(UObject* owner)
        {
            Callbacks.RemoveAll([owner](FCallback& iter) { return iter.Owner == owner; });
        }

        void TickCallback()
        {
            for (int i = 0; i < Callbacks.Num(); ++i)
            {
                Callbacks[i].Callback();
            }
        }
    };
    
public:
    inline static int GActivateLidar = 0;
    inline static int GActivateLidarLast = 0;

    inline static int GActivateRadar = 0;
    inline static int GActivateRadarLast = 0;
    
    inline static int GActivateSceneCaptureCamera = 0;
    inline static int GActivateSceneCaptureCameraLast = 0;
    
    inline static int GActivateGnss = 0;
    inline static int GActivateGnssLast = 0;
    
    inline static int GActivateIMU = 0;
    inline static int GActivateIMULast = 0;

    inline static int GActivateCarlaHUD = 0;

    inline static int GSensorCapturePerspectiveByPrecision = 1;
    inline static int GCameraRTUseFisheye = 0;
    inline static int GRGBCameraRTFisheyePrev = GCameraRTUseFisheye;
    inline static float GFisheyeFov = 140.0f;
    inline static float GFisheyeRelativePosX = -100.0f;

    inline static const float GTimeDeltaReadPixel = 0.1f;
    inline static const int CaptureDivFOVAngle = 3;
    inline static const FIntPoint CaptureRes360 = FIntPoint(2048, 512);
    inline static const FIntPoint LidarCaptureResolution = FIntPoint(CaptureRes360.X / CaptureDivFOVAngle, CaptureRes360.Y);
    inline static const float CaptureFOVAngleUnit = 360.0f / (float)CaptureDivFOVAngle;

    inline static const bool GbVisualizeIntensity = true;
    inline static const float GRatioIntensityCosin = 0.7f;
    inline static const float GRatioIntensityRefle = 1.0f;
    inline static const float GRatioIntensityColor = 0.2f;
    
    inline static TMap<ESensorType, FCallbackCollection> CallbackActivations;
    inline static TMap<ESensorType, FCallbackCollection> CallbackDeactivations;

    inline static TMap<ESensorType, int*> GlobalSensorActivates;

    struct FTMTrack
    {
        double TimeTrack = 0;
        FTransform TMTrack;
        FTMTrack(double timeTrack, const FTransform& tmTrack)
        {
            TimeTrack = timeTrack; TMTrack = tmTrack;
        }
        FTMTrack(){}
    };
    inline static const float TimeLimitTrack = 1.0f;    //sec

public:    
    static void EnsureGlobalFlags()
    {
        if (GlobalSensorActivates.Num() <= 0)
        {
            GlobalSensorActivates.Add(ESensorType::Lidar, &GActivateLidar);
            GlobalSensorActivates.Add(ESensorType::Radar, &GActivateRadar);
            GlobalSensorActivates.Add(ESensorType::Camera, &GActivateSceneCaptureCamera);
            GlobalSensorActivates.Add(ESensorType::IMU, &GActivateIMU);
            GlobalSensorActivates.Add(ESensorType::GNSS, &GActivateGnss);
        }
    }

    static void SetActivateGlobalSensorAll(bool bActivate = true)
    {
        EnsureGlobalFlags();
        for (TMap<ESensorType, int*>::TIterator iter(GlobalSensorActivates); iter; ++iter)
        {
            ESensorType sensorType = iter.Key();
            SetActivateGlobalSensor(sensorType, bActivate);
        }
    }
    
    static void SetActivateGlobalSensor(ESensorType sensorType, bool bActivate)
    {
        EnsureGlobalFlags();
        ensure(GlobalSensorActivates.Contains(sensorType));
        *(GlobalSensorActivates[sensorType]) = (int)bActivate;
    }

public:
    static void TickCallbackSensors()
    {
        bool bToggledLidar = GActivateLidar != GActivateLidarLast;
        if (bToggledLidar == true && CallbackActivations.Contains(ESensorType::Lidar))
        {
            if (GActivateLidar == 1)
            {
                CallbackActivations[ESensorType::Lidar].TickCallback();
            }
            else
            {
                CallbackDeactivations[ESensorType::Lidar].TickCallback();
            }
        }
        GActivateLidarLast = GActivateLidar;

        bool bToggledRadar = GActivateRadar != GActivateRadarLast;
        if (bToggledRadar == true && CallbackActivations.Contains(ESensorType::Radar))
        {
            if (GActivateRadar == 1)
            {
                CallbackActivations[ESensorType::Radar].TickCallback();
            }
            else
            {
                CallbackDeactivations[ESensorType::Radar].TickCallback();
            }
        }
        GActivateRadarLast = GActivateRadar;

        bool bToggledCamera = GActivateSceneCaptureCamera != GActivateSceneCaptureCameraLast;
        if (bToggledCamera == true && CallbackActivations.Contains(ESensorType::Camera))
        {
            if (GActivateSceneCaptureCamera == 1)
            {
                CallbackActivations[ESensorType::Camera].TickCallback();
            }
            else
            {
                CallbackDeactivations[ESensorType::Camera].TickCallback();
            }
        }
        GActivateSceneCaptureCameraLast = GActivateSceneCaptureCamera;

        bool bToggledIMU = GActivateIMU != GActivateIMULast;
        if (bToggledIMU == true && CallbackActivations.Contains(ESensorType::IMU))
        {
            if (GActivateIMU == 1)
            {
                CallbackActivations[ESensorType::IMU].TickCallback();
            }
            else
            {
                CallbackDeactivations[ESensorType::IMU].TickCallback();
            }
        }
        GActivateIMULast = GActivateIMU;

        bool bToggledGnss = GActivateGnss != GActivateGnssLast;
        if (bToggledGnss == true && CallbackActivations.Contains(ESensorType::GNSS))
        {
            if (GActivateGnss == 1)
            {
                CallbackActivations[ESensorType::GNSS].TickCallback();
            }
            else
            {
                CallbackDeactivations[ESensorType::GNSS].TickCallback();
            }
        }
        GActivateGnssLast = GActivateGnss;
    }

    static void AddCallbackActivateSensor(UObject* owner, ESensorType sensorType, TFunction<void()> callback)
    {
        FCallbackCollection& collection = CallbackActivations.FindOrAdd(sensorType);
        collection.AddCallback(owner, sensorType, callback);
    }

    static void AddCallbackDeactivateSensor(UObject* owner, ESensorType sensorType, TFunction<void()> callback)
    {
        FCallbackCollection& collection = CallbackDeactivations.FindOrAdd(sensorType);
        collection.AddCallback(owner, sensorType, callback);
    }

    static void RemoveCallbackSensor(ESensorType sensorType)
    {
        FCallbackCollection* collection = CallbackActivations.Find(sensorType);
        if (collection != nullptr)
        {
            collection->Callbacks.Empty();
        }

        collection = CallbackDeactivations.Find(sensorType);
        if (collection != nullptr)
        {
            collection->Callbacks.Empty();
        }
    }

    static void StaticInitialize()
    {
    }
};
