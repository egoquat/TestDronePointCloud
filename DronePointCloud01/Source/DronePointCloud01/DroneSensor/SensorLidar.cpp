#include "SensorLidar.h"

#include <cmath>
#include <random>
#include "SensorCapturePool.h"
#include "SensorUtility.h"
#include "Components/LineBatchComponent.h"
#include "Drone/DroneActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"

static int GLidarSaveCaptureTextureRequest = 0;
FAutoConsoleVariableRef CVarSaveCaptureTextureRequest(
TEXT("Binn.Sensor.Lidar.SaveCaptureTextureRequest"),
GLidarSaveCaptureTextureRequest,
TEXT("default Off = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static float GLidarTestFisheyeSimpleDistort = 0.8f;
FAutoConsoleVariableRef CTestFisheyeSimpleDistort(
TEXT("Binn.Sensor.Lidar.TestFisheyeSimpleDistort"),
GLidarTestFisheyeSimpleDistort,
TEXT("default float"),
ECVF_Cheat
);

static int GLidarIntensityUseBinn = 1;
FAutoConsoleVariableRef CVarLidarComputeDetectUseBinn(
TEXT("Binn.Sensor.Lidar.IntensityUseBinn"),
GLidarIntensityUseBinn,
TEXT("default ON = 1\n"),
ECVF_Cheat
);

static int GLidarTestRaycastDrawDirectly = 0;
FAutoConsoleVariableRef CLidarTestRaycastDrawDirectly(
TEXT("Keti.Sensor.Lidar.LidarTestRaycastDrawDirectly"),
GLidarTestRaycastDrawDirectly,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarShowDrawLines = 0;
FAutoConsoleVariableRef CVarGLidarShowDrawLines(
TEXT("Keti.Sensor.Lidar.ShowDrawLines"),
GLidarShowDrawLines,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarVisualizeSemantic = 0;
FAutoConsoleVariableRef CVarVisualizeSemantic(
TEXT("Keti.Sensor.Lidar.VisualizeSemantic"),
GLidarVisualizeSemantic,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarVisualize360 = 1;
FAutoConsoleVariableRef CVarVisualize360(
TEXT("Keti.Sensor.Lidar.Visualize360"),
GLidarVisualize360,
TEXT("default ON = 1\n")
TEXT("OFF = 0\n"),
ECVF_Cheat
);

static int GLidarShowVisualizeGameWorld = 1;
FAutoConsoleVariableRef CVarShowVisualizeGameWorld(
TEXT("Keti.Sensor.Lidar.ShowVisualizeGameWorld"),
GLidarShowVisualizeGameWorld,
TEXT("defaul ON = 1\n"),
ECVF_Cheat
);

static const float SecondDrawDefault = 0.2f;
static const float DistanceDotSt = 10000;

typedef FSensorCommon::FTMTrack FTMTrack;
typedef FSensorUtility::FProfiler FProfiler;

ASensorLidar::ASensorLidar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
}

void ASensorLidar::BeginPlay()
{
    Super::BeginPlay();
    
	UpdateDescriptionToThisSensor();
	
	TMSensorTracks.Add(FTMTrack(FPlatformTime::Seconds(), TMSensorAroundViewLast));
	
    SetActorEnableCollision(false);
    
    UStaticMeshComponent* meshComp = Cast<UStaticMeshComponent>(RootComponent);
    if (meshComp)
    {
        meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        //meshComp->SetHiddenInGame(true);
    }
    
    float pitchMid = (ActiveDescription.LowerFovLimit + ActiveDescription.UpperFovLimit) / 2;
    AroundCaptures.Empty();
    float yawLookAt = 0;
    float pitchLookAt = pitchMid;
    FRotator rotatorCaptureLookAt(pitchLookAt, yawLookAt, 0);
    for (int i = 0; i < FSensorCommon::CaptureDivFOVAngle; ++i)
    {
        UTextureRenderTarget2D* renderTargetNew;
        rotatorCaptureLookAt.Yaw = ((float)i * FSensorCommon::CaptureFOVAngleUnit);
        USceneCaptureComponent2D* captureCompNew = FSensorCapture::MakeSceneCaptureRT(this,
                                                rotatorCaptureLookAt, FSensorCommon::CaptureFOVAngleUnit,
                                                FSensorCommon::LidarCaptureResolution.X,
                                                FSensorCommon::LidarCaptureResolution.Y,
                                                true, renderTargetNew);
        AroundCaptures.Add(FSensorCapture(i, FSensorCommon::LidarCaptureResolution,
                                        FSensorCommon::CaptureFOVAngleUnit,
                                        rotatorCaptureLookAt.Yaw,
                                        pitchLookAt,
                                        captureCompNew,
                                        renderTargetNew));
        FSensorCapturePool::AddCapture(this, captureCompNew);
    }

#if TestCaptureON == 1
    TestCaptureSingle = FSensorCapture(0, FSensorCommon::LidarCaptureResolution,
                                            FSensorCommon::CaptureFOVAngleUnit, 0, pitchMid, nullptr, nullptr);
    TestCaptureSingle.CaptureComp = FSensorCapture::MakeSceneCaptureRT(this,
                                    rotatorCaptureLookAt, FSensorCommon::CaptureFOVAngleUnit,
                                    FSensorCommon::LidarCaptureResolution.X,
                                    FSensorCommon::LidarCaptureResolution.Y, true, TestCaptureSingle.RenderTarget);
    TestCaptureSingle.OnUpdateCapture();
#endif

    AngleHorizontalCurrent = AngleHorizontalStart = 0.0f;
    AngleHAbsCache = AngleDraw360CacheStart = AngleDraw360Cache = 0.0f;

    TraceParams.bTraceComplex = true;
    TraceParams.bReturnPhysicalMaterial = true;
    TraceParams.bReturnFaceIndex = false;
    TraceParams.AddIgnoredActor(this);
    
    Bitmap.Empty();
    Bitmap.AddZeroed(FSensorCommon::LidarCaptureResolution.X * FSensorCommon::LidarCaptureResolution.X);

    UWorld* world = GetWorld();
    
	TMSensorAroundViewLast = TMSensorTracks.Last().TMTrack;
}

void ASensorLidar::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearLidar();
}

void ASensorLidar::RecreateLaserAngles() 
{    
	const int32 NumberOfLasers = ActiveDescription.ChannelAngles.Num();
	float angleDelta = 0;
	float angleSum   = 0;
	LaserAngles.SetNum(NumberOfLasers, true);
    
	float ratio = 0, lastratio = 0;
	float angleRangeVertical = (ActiveDescription.UpperFovLimit - ActiveDescription.LowerFovLimit);
	    
	for (int32 i = 0; i < NumberOfLasers; ++i)
	{
		ratio = ((float)i / (float)(NumberOfLasers - 1));
		angleDelta = ActiveDescription.ChannelAngles[i];
		angleSum = angleSum + angleDelta;
		const float VerticalAngle = ActiveDescription.LowerFovLimit + angleSum;
		LaserAngles[NumberOfLasers - i - 1] = (VerticalAngle);
	}
}

void ASensorLidar::RecreateChannelAngles()
{
	ActiveDescription.ChannelAngles.Init(0, ActiveDescription.Channels);
    
	const auto NumberOfLasers = ActiveDescription.Channels;
	check(NumberOfLasers > 0u);
	const float DeltaAngle = NumberOfLasers == 1u ? 0.f : (ActiveDescription.UpperFovLimit - ActiveDescription.LowerFovLimit) /
	  static_cast<float>(NumberOfLasers - 1);
    
	for(auto i = 0u; i < NumberOfLasers; ++i)
	{
		ActiveDescription.ChannelAngles[i] = DeltaAngle;
	}
}

void ASensorLidar::UpdateDescriptionToThisSensor()
{
	ResolutionHorizontal = ActiveDescription.ResolutionHorizontal;
	AngleUnitHorizontalLidar = 360.0f / ResolutionHorizontal;
	AngleHorizontalStart = 0;
    
	PointsPerChannel.resize(ActiveDescription.Channels);
	
	RecreateChannelAngles();
	RecreateLaserAngles();
	
	CountTotalLaser = ResolutionHorizontal * ActiveDescription.Channels;

	FCaptureRay defaultRay;
	DrawCircleRingRays.Init(defaultRay, CountTotalLaser);
	DrawUIRayRingPoints.Init(FVector2D::ZeroVector, CountTotalLaser);
	DrawUIRayRingColors.Init(FColor::Black, CountTotalLaser);
	IndexRing = CountAroundRing = CountLaserRTRayBuffer = 0;

	Index360Capacity = CountTotalLaser;
	Index360Iterator = 0;
	DrawCircleRays.SetNum(Index360Capacity);
	DrawUIRayPoints.SetNum(Index360Capacity);
	DrawUIRayColors.SetNum(Index360Capacity);
}

void ASensorLidar::ClearLidar()
{
	TestCaptureRays.Empty();
	for (int i = 0; i < AroundCaptures.Num(); ++i)
	{
		AroundCaptures[i].CollectRays.Empty();
	}
	TestCaptureSingle.CollectRays.Empty();
	LidarRecordedHits.clear();    
	TraceParams.ClearIgnoredActors();
}

FCaptureRay* ASensorLidar::ArrangeRayToAroundCaptureNew(FHitResult& hit, FTransform& tmSensor,
	TArray<FSensorCapture>& aroundCaptures, int32& indexpack_out)
{
	FRotator rotLaser = (hit.TraceEnd - hit.TraceStart).Rotation();
	FSensorCapture* capture = aroundCaptures.FindByPredicate(
			[&](FSensorCapture& iter)
				{ return iter.IsAngleYawRange(rotLaser.Yaw); });
	ensure(capture);
	if (capture == nullptr) return nullptr;
    
	return capture->GetArrangedRay(hit, indexpack_out);
}

void ASensorLidar::TickCaptureAroundRotation(UWorld* World, const float DeltaTime)
{
    FProfiler::Begin(20);
    //@ 0. Ready capture.
    int cntRayCapture = (int)((float)((ActiveDescription.Channels) * ActiveDescription.ResolutionHorizontal) / FMath::Max((AroundCaptures.Num() - 1), 1));  
    for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
    {
        FSensorCapture& capture = AroundCaptures[icapture];
        capture.CollectRays.SetNum(cntRayCapture);
        capture.IteratorIdxForArrangeRay = 0;
        capture.OnTickInitializeCapture();
    }
    //@ 0. Ready capture. //
    FProfiler::End(20);

    FProfiler::Begin(21);
    //@ 1. Collect Ray
    FTransform tmSensor = GetTransform();
    //FQuat quat(GetActorUpVector(), 90.0f * (3.141592653589f / 180.0f));
    FQuat quat(GetActorUpVector(), 0);
    tmSensor.ConcatenateRotation(quat);
    for (int iChannel = 0; iChannel < LidarRecordedHits.size(); ++iChannel)
    {
        std::vector<FHitResult>& hits = LidarRecordedHits[iChannel];
        for (int iHit = 0; iHit < hits.size(); ++iHit)
        {
            FHitResult& hit = hits[iHit];
            if (hit.bBlockingHit == false) continue;
            int32 indexpack = 0;
            FCaptureRay* captureRay = ArrangeRayToAroundCaptureNew(hit, tmSensor, AroundCaptures, indexpack);
            captureRay->InitializeCaptureRay(hit, tmSensor);
            hit.Item = indexpack;
        }
    } // for (int iChannel = 0; iChannel < LidarRecordedHits.size(); ++iChannel)
    //@ 1. Collect Ray //
    FProfiler::End(21);

    FProfiler::Begin(22);
    //@ 2. Capture Scene Render Texture
    for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
    {
        FSensorCapture& capture = AroundCaptures[icapture];
        if (capture.CollectRays.Num() <= 0) continue;

        capture.CaptureRequestWithLinear();
    } // for (int i = 0; i < AroundCaptures.Num(); ++i)
    //@ 2. Capture Scene Render Texture //
    FProfiler::End(22);
}

void ASensorLidar::TickCapture(UWorld* World, const float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(STAT_LidarTickCapture);
	TickCaptureAroundRotation(World, DeltaTime);
    
	if (GLidarSaveCaptureTextureRequest == 1)
	{
		for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
		{
			FSensorCapture& capture = AroundCaptures[icapture];
			FString pathSaveTextureTest = FString::Printf(TEXT("Captured_Test_360_%d.bmp"), icapture);
            
			FSensorUtility::SaveTextureToDiskPixels(capture.Res, capture.Bitmap, pathSaveTextureTest);
#if CaptureLinearColor == 1
			FSensorUtility::SavePixelsToDiskDepthTest(capture.Res, capture.BitmapLinear, pathSaveTextureTest);
#endif

			TArray<FColor> fisheyeBits;
			FSensorUtility::MakeDistortFisheyeSimple(capture.Res.X, capture.Res.Y, GLidarTestFisheyeSimpleDistort,
															capture.Bitmap, fisheyeBits);
            
			pathSaveTextureTest = FString::Printf(TEXT("Captured_Test_360_fisheye_%d.bmp"), icapture);
			FSensorUtility::SaveTextureToDiskPixels(capture.Res, fisheyeBits, pathSaveTextureTest);
		}
		GLidarSaveCaptureTextureRequest = 0;
	}
}

void ASensorLidar::TickCaptureTest(UWorld* World, const float DeltaTime)
{
	
}

bool ASensorLidar::PostprocessDetection(FDetection& Detection) const
{
	if (ActiveDescription.bUseLidarNoise == true && ActiveDescription.NoiseStdDev > std::numeric_limits<float>::epsilon()) {
		const auto ForwardVector = Detection.point.GetSafeNormal();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::normal_distribution<double> d(0.0, ActiveDescription.NoiseStdDev);
		double normaldistribution = d(gen);
		const auto Noise = ForwardVector * normaldistribution;
		Detection.point += Noise;
	}

	const float Intensity = Detection.intensity;
	if(Intensity > ActiveDescription.DropOffIntensityLimit)
	{
		return true;
	}
	else
	{
		float DropOffAlpha = ActiveDescription.DropOffAtZeroIntensity / ActiveDescription.DropOffIntensityLimit;
		float DropOffBeta = 1.0f - ActiveDescription.DropOffAtZeroIntensity;
		return FMath::FRandRange(0.0f, 1.0f)  < DropOffAlpha * Intensity + DropOffBeta;
	}
}

bool ASensorLidar::PostprocessDetectionForBinn(FDetection& Detection, const FHitResult& hit, float range, FVector& noise_out)
{
	// Tick Noise 
    if (ActiveDescription.bUseLidarNoise == true)
    {
        float distance = hit.Distance;
        const float dist = FMath::Clamp(distance * 0.01f, 0.0f, 300.0f);
        // 참고: https://data.ouster.io/downloads/datasheets/datasheet-rev7-v2p5-os2.pdf
        // 참고: https://www.wolframalpha.com/input?i=2.31575756e-04*x%5E2+%2B+-5.70213303e-03*x%5E1+%2B+1.82812720e%2B00%2C+x%3D0+to+x%3D300
        //2.31575756e-04x^2 + -5.70213303e-03x^1 + 1.82812720e+00
        const double stdDevEstimate = (ActiveDescription.NoiseCo2Curve * FMath::Pow(dist, 2))
                                    + (ActiveDescription.NoiseCo1Curve * dist)
                                    + ActiveDescription.NoiseCo0Curve;
        const float noiseStdDev = stdDevEstimate * 0.01f * 1.0f;
    	std::random_device rd;
    	std::mt19937 gen(rd());
    	std::normal_distribution<double> d(0.0, noiseStdDev);
    	double normaldistribution = d(gen);
        const float noiseDistribution = normaldistribution * 0.5f;
        
        // float xRand = FMath::RandRange(-noiseDistribution, noiseDistribution);
        // float yRand = FMath::RandRange(-noiseDistribution, noiseDistribution);
        // float zRand = FMath::RandRange(-noiseDistribution, noiseDistribution);
        // Vector3D noise(xRand, yRand, zRand);
        // Detection.point += noise;

        FVector direction = Detection.point;
        float len = direction.Length();
        direction /= len;
        float rangeError = FMath::RandRange(-noiseDistribution, noiseDistribution);
        Detection.point += (direction * rangeError);
        noise_out = Detection.point;

        //GLog->Logf(TEXT("distance:%f, noiseStdDev:%f"), distance, noiseStdDev);
    }
    // Tick Noise //

    //// Mean Range Error
    //if (ActiveDescription.NoiseMeanRangeError > KINDA_SMALL_NUMBER)
    //{
    //    float meanRangeError = ActiveDescription.NoiseMeanRangeError * 0.5f;
    //    Vector3D direction = Detection.point;
    //    float len = direction.Length();
    //    direction /= len;
    //    float rangeError = FMath::RandRange(-meanRangeError, meanRangeError) * 0.01f;
    //    Detection.point += (direction * rangeError);
    //}
    //// Mean Range Error //
    
    //// Tick Noise 
    //if (ActiveDescription.NoiseStdDev > std::numeric_limits<float>::epsilon()) {
    //    const Vector3D ForwardVector = Detection.point.MakeUnitVector();
    //    const float ratioNoise = (GLidarWeightNoise == 1 ? (FMath::Pow(FMath::Min(distance, range) / range, 0.5f)) : 1.0f); 
    //    const float noiseDistribution = RandomEngine->GetNormalDistribution(0.0f, ActiveDescription.NoiseStdDev)
    //                * ratioNoise;
    //    const Vector3D noise = ForwardVector * noiseDistribution;
    //    Detection.point += noise;
    //    memcpy(&noise_out, &noise, sizeof(Vector3D));
    //}
    //// Tick Noise //

    const float Intensity = Detection.intensity;
    if(Intensity > ActiveDescription.DropOffIntensityLimit)
    {
        return true;
    }
    else
    {
    	float DropOffAlpha = ActiveDescription.DropOffAtZeroIntensity / ActiveDescription.DropOffIntensityLimit;
    	float DropOffBeta = 1.0f - ActiveDescription.DropOffAtZeroIntensity;
        return FMath::FRandRange(0.0f, 1.0f) < DropOffAlpha * Intensity + DropOffBeta;
    }
}

FDetection ASensorLidar::ComputeDetectionSingleCarla(const FHitResult& HitInfo, const FTransform& SensorTransf) const
{
	FDetection Detection;
	const FVector HitPoint = HitInfo.ImpactPoint;
	Detection.point = SensorTransf.Inverse().TransformPosition(HitPoint);

	const float Distance = Detection.point.Length();

	const float AttenAtm = ActiveDescription.AtmospAttenRate;
	const float AbsAtm = exp(-AttenAtm * Distance * 0.01f);

	const float IntRec = AbsAtm;

	Detection.intensity = IntRec;
	return Detection;
}

void ASensorLidar::ComputeDetectionSingleBinn(const FHitResult& HitInfo, const FTransform& SensorTransf, FDetection& detection_out) const
{
	const FVector HitPoint = HitInfo.ImpactPoint;
	detection_out.point  = SensorTransf.Inverse().TransformPosition(HitPoint);

	const float Distance = detection_out.point.Length();

	//거리에 따른 대기 감쇄
	const float AttenAtm = ActiveDescription.AtmospAttenRate;
	const float AbsAtm   = exp(-AttenAtm * Distance * 0.01f);

	const FVector VecInc = -(HitPoint - SensorTransf.GetLocation()).GetSafeNormal();
	const float   cosTh  = FVector::DotProduct(VecInc, HitInfo.ImpactNormal);
    
	uint32_t              object_tag = HitInfo.Component != nullptr ? static_cast<uint32_t>(HitInfo.Component->CustomDepthStencilValue) : 0;

	float matRef = Reflectivities.Contains(object_tag) ? Reflectivities[object_tag] : 0.5f;
	float colorintensity = 1.0f;
	float r, g, b, a;
	FSensorCapture::GetUnpackLinearColorRGBA(HitInfo.Item, r, g, b, a);
	colorintensity = FSensorUtility::GetLightColorIntensity(r, g, b);
    
	const float IntRec = ((AbsAtm * ActiveDescription.RatioIntensityAtmos) + (1.0f - ActiveDescription.RatioIntensityAtmos))
						* ((cosTh * ActiveDescription.RatioIntensityCosin) + (1.0f - ActiveDescription.RatioIntensityCosin))
						* ((matRef * ActiveDescription.RatioIntensityRefle) + (1.0f - ActiveDescription.RatioIntensityRefle))
						* ((colorintensity * ActiveDescription.RatioIntensityColor) + (1.0f - ActiveDescription.RatioIntensityColor));
	detection_out.intensity = IntRec;
}

void ASensorLidar::ComputeAndSaveDetectionsCarla(const FTransform& SensorTransform)
{
	for (auto idxChannel = 0u; idxChannel < ActiveDescription.Channels; ++idxChannel)
		PointsPerChannel[idxChannel] = LidarRecordedHits[idxChannel].size();

	int idxCapture = 0, idxRay = 0;
	bool bDetected = false;
	for (auto idxChannel = 0u; idxChannel < ActiveDescription.Channels; ++idxChannel) {
		for (FHitResult& hit : LidarRecordedHits[idxChannel]) {
			FDetection Detection = ComputeDetectionSingleCarla(hit, SensorTransform);
			idxCapture = hit.Item & 0xff;
			idxRay = (hit.Item >> 8) & 0xffffff;
			bDetected = PostprocessDetection(Detection); 
			if (bDetected == true)
			{
			}
			else
			{
				PointsPerChannel[idxChannel]--;
			}   
			AroundCaptures[idxCapture].CollectRays[idxRay].SetDetectResult(Detection.intensity, Detection.point, SensorTransform, bDetected);
		}
	}
}

void ASensorLidar::ComputeAndSaveDetectionsBinn(const FTransform& SensorTransform)
{
	const uint32 channels = ActiveDescription.Channels;
    for (auto idxChannel = 0u; idxChannel < channels; ++idxChannel)
        PointsPerChannel[idxChannel] = LidarRecordedHits[idxChannel].size();
    
    TAtomic<uint32> CompletedCount(0);
    ParallelFor(channels, [this, &SensorTransform = SensorTransform, &CompletedCount = CompletedCount](int32 idxChannel) {
    //for (auto idxChannel = 0u; idxChannel < channels; ++idxChannel)
    {
        FVector noise;
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
    
            FDetection Detection;
            ComputeDetectionSingleBinn(hit, SensorTransform, Detection);
    
            bool bDetected = PostprocessDetectionForBinn(Detection, hit, ActiveDescription.RangeMax, noise);
            if (bDetected == true)
            {
                CountingWriteTest++;
            }
            else
            {
                PointsPerChannel[idxChannel]--;
            }
            AroundCaptures[idxCapture].CollectRays[idxRay].SetDetectResult(
                    Detection.intensity,Detection.point, SensorTransform, bDetected, FSensorCommon::IntensityColors);
        }

        CompletedCount.IncrementExchange();
    }
    });

    while (CompletedCount.Load() < channels)
    {
        // Just wait for completion parallel_for
    }
}

void ASensorLidar::ComputeAndSaveDetections(const FTransform& SensorTransform)
{
	if (GLidarIntensityUseBinn == 1)
	{
		ComputeAndSaveDetectionsBinn(SensorTransform);
	}
	else
	{
		ComputeAndSaveDetectionsCarla(SensorTransform);
	}
}

void ASensorLidar::TickSensor(const float DeltaTime)
{
	UWorld* world = GetWorld();
	SimulateLidarAround(world, DeltaTime);
	TickCapture(world, DeltaTime);
	
	if (LidarRecordedHits.size() >= 1)
	{
		FTransform ActorTransf = GetTransform();
		ComputeAndSaveDetections(ActorTransf);
	}
	
	TickVisualize(world, DeltaTime);
}

void ASensorLidar::TickVisualizeByLineBatcher(UWorld* World, const float DeltaTime)
{
	ULineBatchComponent* LineBatcher = World->GetLineBatcher(UWorld::ELineBatcherType::World);

    APlayerController *pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    ensure(pc);
    ensure(pc->PlayerCameraManager);
    FVector posCamera = pc->PlayerCameraManager->GetCameraLocation();
    
    if (GLidarTestRaycastDrawDirectly == 1)
    {
        for (int i = 0; i < LidarRecordedHits.size(); ++i)
        {
            std::vector<FHitResult>& hits = LidarRecordedHits[i];
            for (int j = 0; j < hits.size(); ++j)
            {
                FHitResult& hit = hits[j];
                LineBatcher->DrawPoint(hit.ImpactPoint, FColor::Cyan, ActiveDescription.SizeBaseDots, FSensorUtility::DepthPriorityDraw, SecondDrawDefault);
            }
        }
        return;
    }
    
    if (TestCaptureRays.Num() >= 1)
    {
        if (GLidarShowDrawLines != 0)
        {
            FVector locStartCapture = TestCaptureSingle.CaptureComp->GetComponentLocation();
            FVector locEndCapture = locStartCapture + (TestCaptureSingle.CaptureComp->GetForwardVector() * 500);
            LineBatcher->DrawPoint(locStartCapture, FLinearColor::Blue, 20.0f, SDPG_Foreground);
            LineBatcher->DrawLine(locStartCapture, locEndCapture, FLinearColor::Red, SDPG_Foreground, 2.0f, ActiveDescription.SizeBaseDots);
            
            for (int i = 1; i < TestCaptureRays.Num(); ++i)
            {
                LineBatcher->DrawLine(TestCaptureRays[i].From, TestCaptureRays[i].Point, FLinearColor(1,1,0, 0.5f), SDPG_Foreground, 1.0f, ActiveDescription.SizeBaseDots);   
            }   
        }

        if (ActiveDescription.bVisualizeIntensity == 1)
        {
            for (int i = 0; i < TestCaptureRays.Num(); ++i)
            {
                FCaptureRay& cast = TestCaptureRays[i];
                LineBatcher->DrawPoint(cast.Point, cast.ColorIntensity, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
            }
        }
        else
        {
            if (GLidarVisualizeSemantic == 0)
            {
                for (int i = 0; i < TestCaptureRays.Num(); ++i)
                {
                    FCaptureRay& cast = TestCaptureRays[i];
                    LineBatcher->DrawPoint(cast.Point, cast.Color, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                }   
            }
            else
            {
                for (int i = 0; i < TestCaptureRays.Num(); ++i)
                {
                    FCaptureRay& cast = TestCaptureRays[i];
                    LineBatcher->DrawPoint(cast.Point, cast.ColorSemantic, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                }
            }   
        }
    }
	
	if (GLidarVisualize360 == 1)
    {
        FTransform tmSensorForCapture = GetTransform();
        FRotator rotSensor = tmSensorForCapture.Rotator();
        rotSensor.Yaw += 90.0f;
        tmSensorForCapture.SetRotation(rotSensor.Quaternion());
        FTransform invtm = tmSensorForCapture.Inverse();
        
        FTransform tmSensor = GetTransform();
        //FRotator rotSensorTrack = rotSensor;
        //rotSensorTrack.Pitch = rotSensorTrack.Roll = 0;
        //tmSensor.SetRotation(rotSensorTrack.Quaternion());
		
        FProfiler::Begin(41);
        int index360 = 0;
        for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
        {
            FSensorCapture& capture = AroundCaptures[icapture];
            for (int i = 0; i < capture.CollectRays.Num(); ++i)
            {
                FCaptureRay& ray = capture.CollectRays[i];
                if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                
                if (Index360Iterator >= Index360Capacity) break;
                index360 = Index360Iterator++;
                ray.UpdateLocalToWorld(tmSensor);
                DrawCircleRays[index360] = ray;
                DrawUIRayPoints[index360] = FVector2D(invtm.TransformPosition(ray.Point));
                //DrawUIRayPoints[index360] = FVector2D(ray.PointLocal);
                DrawUIRayColors[index360] = ActiveDescription.bVisualizeIntensity == true ? ray.ColorIntensity : ray.Color;
            }
            if (Index360Iterator >= Index360Capacity) break;
        }
        FProfiler::End(41);
        
        if ((AngleDraw360Cache - AngleDraw360CacheStart) > 360.0f)
        {
            FProfiler::Begin(42);
            //DrawCircleRays = CacheDrawCircleRays;
            //DrawUIRayPoints = CacheDrawUIRayPoints;
            //DrawUIRayColors = CacheDrawUIRayColors;
    
            AngleDraw360Cache = AngleDraw360CacheStart = AngleHorizontalCurrent;
            Index360Iterator = 0;
            TMSensorAroundViewLast = tmSensor;
            FProfiler::End(42);
        }
        
        // if (GVisualizeTrackToVehicle == 1 && DrawCircleRays.Num() >= 1)
        // {
        //     FProfiler::Begin(43);
        //     if (GLidarDistortionLerp == 1)
        //     {
        //         for (int i = 0; i < DrawCircleRays.Num(); ++i)
        //         {
        //             DrawCircleRays[i].UpdateLocalToWorldLerpOnTrack(TMSensorTracks, TimeNowOnTick);
        //         }
        //     }
        //     else
        //     {
        //         for (int i = 0; i < DrawCircleRays.Num(); ++i)
        //         {
        //             DrawCircleRays[i].UpdateLocalToWorld(tmSensor);
        //         }
        //     }
        //     FProfiler::End(43);
        // }

        if (GLidarShowVisualizeGameWorld == 1)
        {
            if (ActiveDescription.bVisualizeIntensity == true)
            {
                FProfiler::Begin(44);
                for (int i = 0; i < DrawCircleRays.Num(); ++i)
                {
                    FCaptureRay& cast = DrawCircleRays[i];
                    //if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                    LineBatcher->DrawPoint(cast.Point, cast.ColorIntensity, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                    //LineBatcher->DrawPoint(ray.hit.ImpactPoint, ray.ColorIntensity, ActiveDescription.SizeBaseDots, FSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                }
                FProfiler::End(44);
            }
            else
            {
                for (int i = 0; i < DrawCircleRays.Num(); ++i)
                {
                    FCaptureRay& cast = DrawCircleRays[i];
                    //if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                    LineBatcher->DrawPoint(cast.Point, cast.Color, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                    //LineBatcher->DrawPoint(ray.hit.ImpactPoint, ray.Color, ActiveDescription.SizeBaseDots, FSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                }
            }   
        }
    }
    else
    {
        if (ActiveDescription.bVisualizeIntensity == true)
        {
            for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
            {
                FSensorCapture& capture = AroundCaptures[icapture];
                for (int i = 0; i < capture.CollectRays.Num(); ++i)
                {
                    FCaptureRay& cast = capture.CollectRays[i];
                    if (cast.bPicked == false || cast.bIntensityDetected == false) continue;
                    LineBatcher->DrawPoint(cast.Point, cast.ColorIntensity, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                }
            }
        }
        else
        {
            if (GLidarVisualizeSemantic == 0)
            {
                for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
                {
                    FSensorCapture& capture = AroundCaptures[icapture];
                    for (int i = 0; i < capture.CollectRays.Num(); ++i)
                    {
                        FCaptureRay& cast = capture.CollectRays[i];
                        if (cast.bPicked == false) continue;
                        LineBatcher->DrawPoint(cast.Point, cast.Color, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                    }
                }
            }
            else
            {
                for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
                {
                    FSensorCapture& capture = AroundCaptures[icapture];
                    for (int i = 0; i < capture.CollectRays.Num(); ++i)
                    {
                        FCaptureRay& cast = capture.CollectRays[i];
                        if (cast.bPicked == false) continue;
                        LineBatcher->DrawPoint(cast.Point, cast.ColorSemantic, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                    }
                }
            }   
        }   
    }
}

void ASensorLidar::TickVisualize(UWorld* World, const float DeltaTime)
{
	TickVisualizeByLineBatcher(World, DeltaTime);
}

bool ASensorLidar::ShootLaser(  const float VerticalAngle,
                                float HorizontalAngle,
                                FHitResult& HitResult,
                                FCollisionQueryParams& traceParams) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
    FTransform ActorTransf = GetTransform();
    FVector LidarBodyLoc = ActorTransf.GetLocation();
    FRotator LidarBodyRot = ActorTransf.Rotator();

    FRotator LaserRot (VerticalAngle, HorizontalAngle, 0);  // float InPitch, float InYaw, float InRoll
    FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
      LaserRot,
      LidarBodyRot
    );

    const float rangeMin = ActiveDescription.RangeMin;
    const float rangeMax = ActiveDescription.RangeMax;
    
    FVector StartTrace = LidarBodyLoc;
    FVector EndTrace = LidarBodyLoc + (rangeMax * UKismetMathLibrary::GetForwardVector(ResultRot));

    FHitResult HitInfo(ForceInit);
    GetWorld()->LineTraceSingleByChannel(
      HitInfo,
      StartTrace,
      EndTrace,
      ECC_GameTraceChannel2,
      traceParams,
      FCollisionResponseParams::DefaultResponseParam
    );

    const float dist = HitInfo.Distance;
    if (dist < rangeMin)
    {
        HitInfo.bBlockingHit = false;
    }
    
    if (HitInfo.bBlockingHit) {
        HitResult = HitInfo;
        return true;
    }
	
	return false;
}

void ASensorLidar::SimulateLidarAround(const UWorld *world, const float DeltaTime)
{
   FProfiler::CountingRay = FProfiler::CountingRaycast = 0;
    
	const uint32 ChannelCount = ActiveDescription.Channels;
	const uint32 CountHScanALaser =
	FMath::RoundHalfFromZero(
		(ActiveDescription.RotationFrequencyHZ * ActiveDescription.HorizontalFov) * DeltaTime) / AngleUnitHorizontalLidar;
	
	if (CountHScanALaser <= 0)
	{
	    GLog->Logf(TEXT("%s: no points requested this frame, try increasing the number of points per second."), *GetName());
	    return;
	}
	
	check(ChannelCount == LaserAngles.Num());
	
	const float CurrentHorizontalAngle = AngleHorizontalCurrent;
	const float AngleHoriDistUnitLaser = AngleUnitHorizontalLidar;
    const float AngleHoriFOV           = ActiveDescription.HorizontalFov;
    const float AngleHoriFOVSkipRange  = 360.0f - AngleHoriFOV;
    const float AngleHoriFOVSkipStart  = AngleHoriFOV * 0.5f;
	
	ResetLidarRecordedHits(ChannelCount, CountHScanALaser);

    int countActualScan = 0;

	FPhysScene* physScene = world->GetPhysicsScene();
	ensure(physScene);
	
	Chaos::FPhysicsSolver* chaosScene = physScene->GetSolver();
	ensure(chaosScene);
	
	{
   		FScopedSceneLock_Chaos sceneReadLock(physScene, EPhysicsInterfaceScopedLockType::Read);
   		ParallelFor(ChannelCount, [&](int32 idxChannel ) 
   		{
			for (auto idxPtsOneLaser = 0u; idxPtsOneLaser < CountHScanALaser; idxPtsOneLaser++)
			{
				const float VertAngle = LaserAngles[idxChannel];
				const float HorizAngleRelative = AngleHoriDistUnitLaser * idxPtsOneLaser;
				const float HorizAngle = std::fmod(CurrentHorizontalAngle + HorizAngleRelative, 360.0f);
				if (AngleHoriFOV < (360.0f - 0.1f) 
				   && (HorizAngle >= AngleHoriFOVSkipStart && HorizAngle <= (AngleHoriFOVSkipStart + AngleHoriFOVSkipRange)))
				{
				   continue;
				}

				FProfiler::CountingRay++;
				FHitResult HitResult;
				if (ShootLaser(VertAngle, HorizAngle, HitResult, TraceParams))
				{
					FProfiler::CountingRaycast++;

					//@ distortion 01 
					float ratioYaw = HorizAngle / 360.0f;
					HitResult.FaceIndex = (int32)((double)FSensorCommon::MaxInt * ratioYaw);
					//@ distortion 02
					double angleHAbsCurrent = (AngleHAbsCache + (HorizAngleRelative / 360.0));
					HitResult.Time = angleHAbsCurrent;
					LidarRecordedHits[idxChannel].emplace_back(HitResult);
					countActualScan++;
				}
			}
		});
	}

    CountAroundRing = (CountAroundRing + countActualScan) % CountTotalLaser; 
    
    const float AngleDistanceOfTick = AngleUnitHorizontalLidar * (CountHScanALaser);
    FRotator laserRotYaw(0, CurrentHorizontalAngle, 0);
	const float HorizontalAngle = std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, 360.0f);
    AngleHorizontalCurrent = HorizontalAngle;
    AngleDraw360Cache += AngleDistanceOfTick;
    AngleHAbsCache += (AngleDistanceOfTick / 360.0f);
}

void MakeOneLaserWorldDirection(const FRotator& sensorRot, const float angleV, float angleH, FVector3f& laser_out)
{
	FRotator laserRot (angleV, angleH, 0);
	laser_out = FVector3f(laserRot.Vector());
	//FRotator resultRot = UKismetMathLibrary::ComposeRotators(
	//  laserRot,
	//  sensorRot
	//);
	//laser_out = FVector3f(resultRot.Vector());
}

void ASensorLidar::ResetLidarRecordedHits(uint32_t Channels, uint32_t MaxPointsPerChannel)
{
	LidarRecordedHits.resize(Channels);

	for (auto& hits : LidarRecordedHits) {
		hits.clear();
		hits.reserve(MaxPointsPerChannel);
	}
}

void ASensorLidar::ComputeDetectionSingle(const FHitResult& HitInfo, const FTransform& SensorTransf, FDetection& detection_out) const
{
	const FVector HitPoint = HitInfo.ImpactPoint;
	detection_out.point  = SensorTransf.Inverse().TransformPosition(HitPoint);

	const float Distance = detection_out.point.Length();

	//거리에 따른 대기 감쇄
	const float AttenAtm = ActiveDescription.AtmospAttenRate;
	const float AbsAtm   = exp(-AttenAtm * Distance * 0.01f);

	const FVector VecInc = -(HitPoint - SensorTransf.GetLocation()).GetSafeNormal();
	const float   cosTh  = FVector::DotProduct(VecInc, HitInfo.ImpactNormal);
    
	uint32_t              object_tag = HitInfo.Component != nullptr ? static_cast<uint32_t>(HitInfo.Component->CustomDepthStencilValue) : 0;

	float matRef = Reflectivities.Contains(object_tag) ? Reflectivities[object_tag] : 0.5f;
	float colorintensity = 1.0f;
	float r, g, b, a;
	FSensorCapture::GetUnpackLinearColorRGBA(HitInfo.Item, r, g, b, a);
	colorintensity = FSensorUtility::GetLightColorIntensity(r, g, b);
    
	const float IntRec = ((AbsAtm * ActiveDescription.RatioIntensityAtmos) + (1.0f - ActiveDescription.RatioIntensityAtmos))
						* ((cosTh * ActiveDescription.RatioIntensityCosin) + (1.0f - ActiveDescription.RatioIntensityCosin))
						* ((matRef * ActiveDescription.RatioIntensityRefle) + (1.0f - ActiveDescription.RatioIntensityRefle))
						* ((colorintensity * ActiveDescription.RatioIntensityColor) + (1.0f - ActiveDescription.RatioIntensityColor));
	detection_out.intensity = IntRec;
}
