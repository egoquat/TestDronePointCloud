#include "Sensor.h"
#include "Drone/DroneActor.h"

ASensor::ASensor(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
	SensorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorMesh"));
	RootComponent = SensorMesh;
	SensorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ASensor::InitializeSensor(ADroneActor* droneActor)
{
	DroneActor = droneActor;
}

void ASensor::BeginPlay()
{
	Super::BeginPlay();
}

void ASensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ASensor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsActive == false)
		return;
	
	ReadyToTick = true;
	PreTickSensor(DeltaSeconds);
	TickSensor(DeltaSeconds);
	PostTickSensor(DeltaSeconds);
}
