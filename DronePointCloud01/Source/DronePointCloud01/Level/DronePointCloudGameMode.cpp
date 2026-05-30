#include "DronePointCloudGameMode.h"

#include "EngineUtils.h"
#include "Drone/DroneActor.h"

void ADronePointCloudGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* world = GetWorld();
}

void ADronePointCloudGameMode::OnSetActiveDrone(ADroneActor* drone)
{
	for (int i = 0; i < DroneActors.Num(); i++)
	{
		ADroneActor* droneActor = DroneActors[i];
		bool bActiveDrone = droneActor == drone;
		droneActor->SetActiveDrone(bActiveDrone);
	}
}
