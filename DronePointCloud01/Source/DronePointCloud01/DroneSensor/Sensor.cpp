#include "Sensor.h"

ASensor::ASensor(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
}

void ASensor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASensor::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ReadyToTick = true;
	TickSensor(DeltaTime);
}

void ASensor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
}
