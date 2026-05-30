#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DronePointCloudGameMode.generated.h"

UCLASS()
class DRONEPOINTCLOUD01_API ADronePointCloudGameMode : public AGameMode
{
	GENERATED_BODY()
	
	protected:
	virtual void BeginPlay() override;
};
