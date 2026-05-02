#include "SensorLidar.h"

ASensorLidar::ASensorLidar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
}

void ASensorLidar::TickSensor(const float DeltaTime)
{
	
}

void ASensorLidar::TickSensorComputeLidar(const FTransform& SensorTransform)
{
	const uint32 channels = ActiveDescription.Channels;
    for (auto idxChannel = 0u; idxChannel < channels; ++idxChannel)
        PointsPerChannel[idxChannel] = LidarRecordedHits[idxChannel].size();
    
    TAtomic<uint32> CompletedCount(0);
    ParallelFor(channels, [this, &SensorTransform = SensorTransform, &CompletedCount = CompletedCount](int32 idxChannel) {
    //for (auto idxChannel = 0u; idxChannel < channels; ++idxChannel)
    {
        int idxCapture = 0, idxRay = 0;
        for (FHitResult& hit : LidarRecordedHits[idxChannel]) {
            if (hit.bBlockingHit == true)
            {
                idxCapture = hit.Item & 0xff;
                idxRay = (hit.Item >> 8) & 0xffffff;
                FSensorCapture& capture = AroundCaptures[idxCapture];
                FCaptureRay& rayHit = capture.CollectRays[idxRay];
                bool bColorPicked = capture.GetColorFromCapture(
                                                        rayHit.Rot,
                                                        rayHit.Point,
                                                        capture.Bitmap,
                                                        rayHit.Color,
                                                        rayHit.UV);
                if (bColorPicked == true)
                {
                    FColor& captureColor = rayHit.Color;
                    hit.Item = rayHit.Hit.Item = captureColor.ToPackedRGBA();
                }
                rayHit.bPicked = true;
            }
    
    
            PointsPerChannel[idxChannel]--;
            AroundCaptures[idxCapture].CollectRays[idxRay].SetDetectResult(
                    1.0f, hit.ImpactPoint, SensorTransform, true, FSensorCommon::IntensityColors);
        }

        CompletedCount.IncrementExchange();
    }
    });
}
