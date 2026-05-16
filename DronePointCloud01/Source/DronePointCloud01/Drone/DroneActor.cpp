#include "DroneActor.h"

#include "DronePointCloud01/DroneSensor/SensorDescription.h"
#include "DroneSensor/SensorLidar.h"
#include "DroneSensor/DroneSensorDesc/LidarSensorDescJsonParser.h"

ADroneActor::ADroneActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADroneActor::BeginPlay()
{
	Super::BeginPlay();
	
	FLidarSensorDescription lidarSensorDescriptionTest;

	bool bparse = false;
	bparse = FLidarSensorDescJsonParser::LoadFromJsonFile(FilePathSensorLidarDesc, lidarSensorDescriptionTest);
	ensure(bparse);
	
	AddSensorLidar(lidarSensorDescriptionTest);
}

void ADroneActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADroneActor::OnActiveDrone(bool bActive)
{
	for (int i = 0; i < Sensors.Num(); i++)
	{
		Sensors[i]->SetActiveSensor(bActive);
	}
}

void ADroneActor::OnAddSubMeshComponent(UStaticMeshComponent* subsm)
{
	PartStaticMeshComponents.AddUnique(subsm);
}

template <typename SensorType>
SensorType* ADroneActor::AddSensorInternal(const FSensorDescription& desc)
{
	UWorld* world = GetWorld();
	FTransform tmOwner = GetTransform();
	SensorType* sensor  = world->SpawnActorDeferred<SensorType>( SensorType::StaticClass(), tmOwner, this);
	check(sensor);
	if (sensor == nullptr)
	{
		GLog->Logf(ELogVerbosity::Error, TEXT("(sensor == nullptr)"));
		return nullptr;
	}
	
	sensor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	//sensor->SetActorRelativeTransform(desc.TMRelativeSensor);

	const FTransform tmSpawn = desc.TMRelativeSensor * tmOwner;
	sensor->FinishSpawning(tmSpawn);
	sensor->SetActorTickEnabled(true);
	return sensor;
}

ASensorLidar* ADroneActor::AddSensorLidar(const FLidarSensorDescription& desc)
{
	ASensorLidar* sensorLidar = AddSensorInternal<ASensorLidar>(desc);
	ensure(sensorLidar);
	sensorLidar->InitializeSensor(this);
	Sensors.Add(sensorLidar);
	SensorTypes.Add(ESensorType::Lidar, sensorLidar);
	return sensorLidar;
}
