#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DronePointCloudGameMode.generated.h"

class ADroneActor;

UCLASS()
class DRONEPOINTCLOUD01_API ADronePointCloudGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable)
	void OnSetActiveDrone(ADroneActor* drone);
	
public:
	TArray<ADroneActor*> DroneActors;
};
