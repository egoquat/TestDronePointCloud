#pragma once
#include "Sensor.h"
#include <vector>
#include "SensorCapture.h"
#include "SensorDescription.h"
#include "SensorLidar.generated.h"

//@ Const Parameter
static const TMap<int32, float> Reflectivities {
	{0 	, 0.5f},    //None = 0u,          
    {1 	, 0.4f},    //Buildings = 1u,     
    {2 	, 0.7f},    //Fences = 2u,        
    {3 	, 0.5f},   	//Other = 3u,         
    {4 	, 0.5f},   	//Pedestrians = 4u,   
    {5 	, 0.8f},   	//Poles = 5u,         
    {6 	, 1.0f},  	//RoadLines = 6u,     
    {7 	, 0.6f}, 	//Roads = 7u,         
    {8 	, 0.7f}, 	//Sidewalks = 8u,     
    {9 	, 0.7f},  	//Vegetation = 9u,    
    {10 	, 0.7f},  	//Vehicles = 10u,     
    {11 	, 0.4f}, 	//Walls = 11u,        
    {12 	, 1.0f},  	//TrafficSigns = 12u, 
    {13 	, 0.0f},  	//Sky = 13u,          
    {14 	, 0.4f}, 	//Ground = 14u,       
    {15 	, 0.4f}, 	//Bridge = 15u,       
    {16 	, 0.6f}, 	//RailTrack = 16u,    
    {17 	, 0.9f}, 	//GuardRail = 17u,    
    {18 	, 0.8f},  	//TrafficLight = 18u, 
    {19 	, 0.5f}, 	//Static = 19u,       
    {20 	, 0.5f},  	//Dynamic = 20u,      
    {21 	, 0.1f},  	//Water = 21u,        
    {22 	, 0.2f},  	//Terrain = 22u,      
    {255 	, 0.5f} 	//Any = 0xFF          
};
//@ Const Parameter //

UCLASS()
class ASensorLidar : public ASensor
{
	GENERATED_BODY()
	
public:
	ASensorLidar(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void RecreateLaserAngles();
	void RecreateChannelAngles();
	void UpdateDescriptionToThisSensor();
	void ClearLidar();
	
	FCaptureRay* ArrangeRayToAroundCaptureNew(FHitResult& hit, FTransform& tmSensor,
						TArray<FSensorCapture>& aroundCaptures,
						int32& indexpack_out);
	void TickCaptureAroundRotation(UWorld *World, const float DeltaTime);
	void TickCapture(UWorld *World, const float DeltaTime);
	void TickCaptureTest(UWorld *World, const float DeltaTime);
	
	bool PostprocessDetection(FDetection& Detection) const;
	bool PostprocessDetectionForBinn(FDetection& Detection, const FHitResult& hit, float range, FVector& noise_out);
	
	FDetection ComputeDetectionSingleCarla(const FHitResult& HitInfo, const FTransform& SensorTransf) const;
	void ComputeDetectionSingleBinn(const FHitResult& HitInfo, const FTransform& SensorTransf, FDetection& detection_out) const;
	
	void ComputeAndSaveDetectionsCarla(const FTransform &SensorTransform);
	void ComputeAndSaveDetectionsBinn(const FTransform &SensorTransform);
	
	void ComputeAndSaveDetections(const FTransform& SensorTransform);	

	virtual void TickSensor(const float DeltaTime) override;
	
	void TickVisualizeByLineBatcher(UWorld *World, const float DeltaTime);
	void TickVisualize(UWorld *World, const float DeltaTime);
	
	bool ShootLaser(float VerticalAngle, float HorizontalAngle, FHitResult& HitResult,
	                FCollisionQueryParams& traceParams) const;
	void SimulateLidarAround(const UWorld* world, float DeltaTime);
	void ResetLidarRecordedHits(uint32_t Channels, uint32_t MaxPointsPerChannel);
	void ComputeDetectionSingle(const FHitResult& HitInfo, const FTransform& SensorTransf, FDetection& detection_out) const;
	
protected:
	TArray<float> LaserAngles;
	std::vector<std::vector<FHitResult>> RecordedHits;
	std::vector<std::vector<bool>> RayPreprocessCondition;
	std::vector<uint32_t> PointsPerChannel;
	
	TArray<FSensorCapture> AroundCaptures;
	FSensorCapture TestCaptureSingle;
	TArray<FCaptureRay> TestCaptureRays;
	
	TArray<FCaptureRay> DrawCircleRays;
	TArray<FVector2D> DrawUIRayPoints;
	TArray<FColor> DrawUIRayColors;
	
protected:
	int ResolutionHorizontal        = 1024;  // 10hz
	float AngleUnitHorizontalLidar  = 360.0f / ResolutionHorizontal;
	float AngleHorizontalStart      = 0.0f;
	float AngleHorizontalCurrent    = 0.0f;
	double AngleHAbsCache           = 0.0;
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
	
	int CountingWriteTest = 0;
	FLidarSensorDescription ActiveDescription;
	
protected:
	float AngleDraw360CacheStart = 0.0f;
	float AngleDraw360Cache = AngleHorizontalCurrent;
	int Index360Capacity = ActiveDescription.Channels * ActiveDescription.ResolutionHorizontal; 
	int Index360Iterator = 0;
	std::vector<std::vector<FHitResult>> LidarRecordedHits;

	TArray<FCaptureRay> DrawCircleRingRays;
	TArray<FVector2D> DrawUIRayRingPoints;
	TArray<FColor> DrawUIRayRingColors;
	unsigned int CountTotalLaser = 0;
	unsigned int IndexRing = 0;
	unsigned int CountAroundRing = 0;

	unsigned int IndexLaserRTRayBuffer = 0;
	unsigned int CountLaserRTRayBuffer = 0;
	
	TArray<FColor> Bitmap;
	
	//@ Lidar Distortion
	FTransform TMSensorAroundViewLast;
	TArray<FSensorCommon::FTMTrack> TMSensorTracks;
	double TimeNowOnTick = 0.0;
	//@ Lidar Distortion //
};
