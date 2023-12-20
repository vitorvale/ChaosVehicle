// Code written by Vitor Vale

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "OffroadWheeledVehicleMC.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Physics), meta = (BlueprintSpawnableComponent), hidecategories = (PlanarMovement, "Components|Movement|Planar", Activation, "Components|Activation"))
class OFFROAD_VEHICLE_API UOffroadWheeledVehicleMC : public UChaosWheeledVehicleMovementComponent
{
	GENERATED_BODY()
	

	TUniquePtr<Chaos::FSimpleWheeledVehicle> CreatePhysicsVehicle() override;

};
