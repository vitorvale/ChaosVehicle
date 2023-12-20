// Code written by Vitor Vale


#include "OffroadWheeledVehicleMC.h"
#include "OffroadWheeledVehicleSimulation.h"

TUniquePtr<Chaos::FSimpleWheeledVehicle> UOffroadWheeledVehicleMC::CreatePhysicsVehicle()
{
	VehicleSimulationPT = MakeUnique<OffroadWheeledVehicleSimulation>();

	return UChaosVehicleMovementComponent::CreatePhysicsVehicle();
}
