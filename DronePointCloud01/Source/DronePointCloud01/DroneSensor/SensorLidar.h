#pragma once
#include "Sensor.h"
#include "SensorLidar.generated.h"

UCLASS(Abstract, hidecategories = (Collision, Attachment, Actor))
class ASensorLidar : public ASensor
{
	GENERATED_BODY()
	
public:
	ASensorLidar(const FObjectInitializer& ObjectInitializer);

	virtual void TickSensor(const float DeltaTime);
};
