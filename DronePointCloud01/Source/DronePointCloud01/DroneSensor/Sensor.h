#pragma once
#include "GameFramework/Actor.h"
#include "Sensor.generated.h"

UCLASS(Abstract, hidecategories = (Collision, Attachment, Actor))
class ASensor : public AActor
{
  GENERATED_BODY()

public:

  ASensor(const FObjectInitializer &ObjectInitializer);

  virtual void BeginPlay();
  
protected:
  void Tick(const float DeltaTime);
  void EndPlay(EEndPlayReason::Type EndPlayReason) override;
  
  virtual void TickSensor(const float deltaSecond) {}

  UPROPERTY()
  bool bIsActive = false;

private:
  bool ReadyToTick = false;
};
