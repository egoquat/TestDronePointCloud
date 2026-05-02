#pragma once
#include "Sensor.h"
#include <vector>
#include "SensorCapture.h"
#include "LidarDescription.h"
#include "SensorLidar.generated.h"

UCLASS(Abstract, hidecategories = (Collision, Attachment, Actor))
class ASensorLidar : public ASensor
{
	GENERATED_BODY()
	
public:
	ASensorLidar(const FObjectInitializer& ObjectInitializer);

	virtual void TickSensor(const float DeltaTime);
	
	void TickSensorComputeLidar(const FTransform &SensorTransform);
	
protected:
	TArray<float> LaserAngles;
	std::vector<std::vector<FHitResult>> RecordedHits;
	std::vector<std::vector<bool>> RayPreprocessCondition;
	std::vector<uint32_t> PointsPerChannel;
	
	TArray<FSensorCapture> AroundCaptures;
	
protected:
	int ResolutionHorizontal        = 2048;  // 10hz
	float AngleUnitHorizontalLidar  = 360.0f / ResolutionHorizontal;
	float AngleHorizontalStart      = 0.0f;
	float AngleHorizontalCurrent    = 0.0f;
	double AngleHAbsCache           = 0.0;
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
	
	int CountingWriteTest = 0;
	FLidarDescription ActiveDescription;
	
protected:
	float AngleDraw360CacheStart = 0.0f;
	float AngleDraw360Cache = AngleHorizontalCurrent;
	int Index360Capacity = ActiveDescription.Channels * ActiveDescription.ResolutionHorizontal; 
	int Index360Iterator = 0;
	std::vector<std::vector<FHitResult>> LidarRecordedHits;
};
