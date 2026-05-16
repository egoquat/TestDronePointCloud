#include "SensorCaptureRay.h"
#include "Components/LineBatchComponent.h"
#include "SensorUIVisualizer.h"

FColor FCaptureRay::GetColorSemanticTag(uint8_t object_tag)
{   
    FColor colorSemantic;    
    const uint8_t* colorSemanticPalette = FSensorCommon::CITYSCAPES_PALETTE_MAP[object_tag];
    colorSemantic.R = colorSemanticPalette[0];
    colorSemantic.G = colorSemanticPalette[1];
    colorSemantic.B = colorSemanticPalette[2];
    colorSemantic.A = 255;
    return colorSemantic;
}

float FCaptureRay::GetSizeDot(const FVector& posCamera, float distanceSt, float sizeDotSt)
{
    //const static float MaxDist = 50000.0f;
    //const static float MinDist = 10.0f;
    //const static float MaxSize = 10.0f;
    //const static float MinSize = 0.2f;
    //float sizeCurrent = (distanceSt / distCamera * sizeDotSt);
    //sizeCurrent = (MinSize + ((distCamera - MinDist) / (MaxDist - MinDist) * (MaxSize - MinSize))) * sizeDotSt;
    //sizeCurrent = FMath::Clamp(sizeCurrent, MinSize, MaxSize);
    float distCamera = (Point - posCamera).Size();
    float sizeCurrent = 1.9f + ((distanceSt / distCamera * sizeDotSt)* 0.6f);
    return sizeCurrent;
}

void FCaptureRay::UpdateLocalToWorldLerpOnTrack(TArray<FSensorCommon::FTMTrack>& tmTracks, double timeNow)
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

void FCaptureRay::UpdateLocalToWorldSonar(TArray<FSensorCommon::FTMTrack>& tmTracks, double angleAbsNow, double timeNowOnTick, int hz)
{
    float angleAbsRay = Hit.Time;
    if (angleAbsNow - angleAbsRay > 1.0) return;
    double ratio = angleAbsNow - angleAbsRay;
    double timeLerp = timeNowOnTick - ((1.0 / hz) * ratio);
    UpdateLocalToWorldLerpOnTrack(tmTracks, timeLerp);
}

void FCaptureRay::SetDetectResult(  const float intensity,
                                    const FVector& worldPosition,
                                    const bool bIntensityDetectedIn)
{
    ColorIntensity = FSensorVisualizer::GetColorLerpRGB(intensity);
    Point = worldPosition;
    PointLocal = worldPosition;
    bIntensityDetected = bIntensityDetectedIn;
}

void FCaptureRay::SetDetectResult(  const float intensity,
                                    const FVector& localPosition,
                                    const FVector& worldPosition,
                                    const bool bIntensityDetectedIn,
                                    const TArray<FColorRatio>& colorTable)
{
    ColorIntensity = FSensorVisualizer::GetColorLerp(intensity, colorTable);
    PointLocal = localPosition;
    Point = worldPosition;
    bIntensityDetected = bIntensityDetectedIn;
}

