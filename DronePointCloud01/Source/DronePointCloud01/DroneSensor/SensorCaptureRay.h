#pragma once

#include "SensorCommon.h"
#include "Engine/EngineTypes.h"
#include "Components/PrimitiveComponent.h"

struct FCaptureRay
{
    FVector From;
    FVector PointWorld;
    FVector PointLocal;
    FRotator Rot;
    FVector DirLocal;
    FColor Color;
    FColor ColorSemantic;
    FColor ColorIntensity;
    FIntPoint UV;
    uint32_t SemanticTag = 0;
    bool bPicked = false;
    bool bIntensityDetected = false;
    FHitResult Hit;
    float RatioOnAround = 0.0f;
    double TimeRayOnHZ = 0.0f;

    static FColor GetColorSemanticTag(uint8_t object_tag);
    
    void Set(   const FVector& from,
                const FVector& point,
                const FRotator& rot,
                const FColor& color,
                const FColor& colorSemantic,
                FIntPoint& uv )
    {
        From = from; PointWorld = point; Rot = rot; Color = color; ColorSemantic = colorSemantic; UV = uv;
        bIntensityDetected = false;
    }

    float GetSizeDot(const FVector& posCamera, float distanceSt, float sizeDotSt);
    void UpdateLocalToWorld(const FTransform& tmSensor)
    {
        PointWorld = tmSensor.TransformPosition(PointLocal);
    }

    void UpdateLocalToWorldDistortAB(const FTransform& tm01, const FTransform& tm02)
    {
        if (RatioOnAround <= KINDA_SMALL_NUMBER)
        {
            UpdateLocalToWorld(tm01);
        }
        else if (RatioOnAround >= 1.0f - KINDA_SMALL_NUMBER)
        {
            UpdateLocalToWorld(tm02);
        }
        else
        {
            FRotator rot01 = tm01.Rotator(), rot02 = tm02.Rotator();
            FVector loc01 = tm01.GetLocation(), loc02 = tm02.GetLocation();
            FRotator rot = FMath::Lerp(rot01, rot02, RatioOnAround);
            FVector loc = FMath::Lerp(loc01, loc02, RatioOnAround);
            FTransform tm(rot);
            tm.SetLocation(loc);
            UpdateLocalToWorld(tm);
        }
    }

    void UpdateLocalToWorldLerpOnTrack(TArray<FSensorCommon::FTMTrack>& tmTracks, double timeNow);
    
    void UpdateLocalToWorldSonar(TArray<FSensorCommon::FTMTrack>& tmTracks, double angleAbsNow, double timeNowOnTick, int hz = 20);

    void UpdateLocalToWorldDistortFakeTrackEllipse(TArray<FSensorCommon::FTMTrack>& tmTracks, double timeNow)
    {
        if (TimeRayOnHZ <= SMALL_NUMBER)
        {
            UpdateLocalToWorld(tmTracks[0].TMTrack);
        }
        else if (TimeRayOnHZ >= 1.0 - SMALL_NUMBER)
        {
            UpdateLocalToWorld(tmTracks.Last().TMTrack);
        }
        else
        {
            FTransform tm;
            GetTransformOnTrack(TimeRayOnHZ, timeNow, tmTracks, tm);
            UpdateLocalToWorld(tm);
        }
    }

    bool GetTransformOnTrack(float timeDistance, double timeNow, TArray<FSensorCommon::FTMTrack>& tmSensorTracks, FTransform& tmOut)
    {
        if (tmSensorTracks.Num() == 1)
        {
            tmOut = tmSensorTracks[0].TMTrack;
            return true;
        }
        double timeDist = FMath::Clamp(timeDistance, 0.0f, 1.0f);
        double t01 = timeNow - tmSensorTracks.Last().TimeTrack;
        double nearest = FMath::Abs(timeDist - t01);
        int nearestIdx = tmSensorTracks.Num() - 1;
        for (int i = tmSensorTracks.Num() - 2; i >= 0; --i)
        {
            FSensorCommon::FTMTrack& trackCurrent = tmSensorTracks[i];
            double t02 = timeNow - trackCurrent.TimeTrack;
            double diff = FMath::Abs(timeDist - t02);
            if (diff < nearest) { nearestIdx = i; nearest = diff; }
            if (t01 <= timeDist && t02 >= timeDist)
            {
                if (FMath::Abs(timeDist - t01) < FMath::Abs(timeDist - t02))
                {
                    tmOut = trackCurrent.TMTrack;
                }
                else
                {
                    tmOut = tmSensorTracks[i+1].TMTrack;
                }

                double ratioUnit = (timeDist - t01) / (t02 - t01);
                ratioUnit = FMath::Clamp(ratioUnit, 0.0, 1.0);
                if (ratioUnit > KINDA_SMALL_NUMBER)
                {
                    FVector loc01 = tmSensorTracks[i+1].TMTrack.GetLocation();
                    FVector loc02 = tmSensorTracks[i].TMTrack.GetLocation();
                    FVector locLerp = FMath::Lerp(loc01, loc02, ratioUnit);
                    tmOut.SetTranslation(locLerp);
                }
                return true;
            }
            t01 = t02;
        }
    
        tmOut = tmSensorTracks[nearestIdx].TMTrack;
        return false;
    }

    void SetDetectResult(   const float intensity,
                            const FVector& worldPosition,
                            const bool bIntensityDetectedIn);
    
    void SetDetectResult(   const float intensity,
                            const FVector& localPosition,
                            const FVector& worldPosition,
                            const bool bIntensityDetectedIn,
                            const TArray<FColorRatio>& intensityColorTable);

    void InitializeCaptureRay(FHitResult& hitResult, const FTransform& tmSensor, float hz = 20)
    {        
        Hit = hitResult;
        FColor colorSemantic;
        uint32_t object_tag = 0; 
        if (hitResult.bBlockingHit == true && hitResult.Component != nullptr)
        {
            object_tag = hitResult.Component->CustomDepthStencilValue;
        }
        colorSemantic = GetColorSemanticTag(object_tag);
        SemanticTag = object_tag;
        FIntPoint uv;
        hitResult.Item = 0;
        Set(hitResult.TraceStart, hitResult.ImpactPoint, (hitResult.TraceEnd - hitResult.TraceStart).Rotation(), FColor(), colorSemantic, uv);
        PointLocal = tmSensor.InverseTransformPosition(PointWorld);
        DirLocal = PointWorld; DirLocal.Normalize();
        float ratio = FMath::Clamp(((float)((double)hitResult.FaceIndex / (double)FSensorCommon::MaxInt)), 0.0f, 1.0f);
        ratio = ratio > 0.5f ? ratio - 0.5f : ratio + 0.5f;
        RatioOnAround = FMath::Clamp(ratio, 0.0f, 1.0f);
        TimeRayOnHZ = (1.0 / hz) * RatioOnAround;
    }
    
    explicit FCaptureRay(FHitResult& hitResult, const FTransform& tmSensor, float hz = 20)
    {        
        InitializeCaptureRay(hitResult, tmSensor, hz);
    }
    
    explicit FCaptureRay( const FVector& from,
                    const FVector& location,
                    const FRotator& rot,
                    const FColor& color,
                    const FColor& colorSemantic,
                    FIntPoint& uv )
    {
        Set(from, location, rot, color, colorSemantic, uv);
    }

    explicit FCaptureRay() {}
};
