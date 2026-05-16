#pragma once
#include "GameFramework/Actor.h"
#include "SensorCommon.h"
#include "Sensor.generated.h"

class ADroneActor; 

UCLASS(Abstract)
class ASensor : public AActor
{
	GENERATED_BODY()

public:
	ASensor(const FObjectInitializer &ObjectInitializer);
	virtual void InitializeSensor(ADroneActor* droneActor);
	void SetActiveSensor(bool bActive = true) { bIsActive = bActive; }
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	virtual void Tick(float DeltaSeconds) override;
	
protected:
	virtual void TickSensor(const float deltaSecond) {}
	virtual void PreTickSensor(const float DeltaTime) {}
	virtual void PostTickSensor(const float DeltaTime) {}

protected:
	UPROPERTY()
	ESensorType SensorType = ESensorType::None;

	UPROPERTY()
	bool bIsActive = true;
	
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SensorMesh;

	ADroneActor* DroneActor = nullptr;

	private:
	bool ReadyToTick = false;
};
