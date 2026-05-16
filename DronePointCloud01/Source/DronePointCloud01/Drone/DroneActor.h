#pragma once

#include "CoreMinimal.h"
#include "DronePointCloud01/DroneSensor/SensorCommon.h"
#include "DronePointCloud01/DroneSensor/SensorDescription.h"
#include "GameFramework/Actor.h"
#include "DroneSensor/SensorLidar.h"
#include "DroneActor.generated.h"

UCLASS()
class DRONEPOINTCLOUD01_API ADroneActor : public APawn
{
	GENERATED_BODY()

public:
	ADroneActor();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void OnActiveDrone(bool bActive);
	
	UFUNCTION(BlueprintCallable)
	void OnAddSubMeshComponent(UStaticMeshComponent* subsm);
	
	const TArray<UStaticMeshComponent*>& GetPartStaticMeshComponents()
	{
		return PartStaticMeshComponents;
	}

private:
	template<typename SensorType>
	SensorType* AddSensorInternal(const FSensorDescription& desc);
protected:
	ASensorLidar* AddSensorLidar(const FLidarSensorDescription& desc);

protected:
	FString FilePathSensorLidarDesc = FPaths::ProjectDir() + TEXT("/UserData/SensorDescLidar01.json");
	
	TArray<ASensor*> Sensors;
	TMap<ESensorType, ASensor*> SensorTypes;
	TArray<UStaticMeshComponent*> PartStaticMeshComponents;
};
