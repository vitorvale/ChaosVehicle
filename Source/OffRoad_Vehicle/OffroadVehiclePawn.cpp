// Code written by Vitor Vale


#include "OffroadVehiclePawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "ChaosVehicleMovementComponent.h"
#include "OffroadWheeledVehicleMC.h"
#include "DisplayDebugHelpers.h"

FName AOffroadVehiclePawn::VehicleMovementComponentName(TEXT("VehicleMovementComp"));
FName AOffroadVehiclePawn::VehicleMeshComponentName(TEXT("VehicleMesh"));

AOffroadVehiclePawn::AOffroadVehiclePawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(VehicleMeshComponentName);
	Mesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	Mesh->BodyInstance.bSimulatePhysics = false;
	Mesh->BodyInstance.bNotifyRigidBodyCollision = true;
	Mesh->BodyInstance.bUseCCD = true;
	Mesh->bBlendPhysics = true;
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCanEverAffectNavigation(false);
	RootComponent = Mesh;

	VehicleMovementComponent = CreateDefaultSubobject<UChaosVehicleMovementComponent, UOffroadWheeledVehicleMC>(VehicleMovementComponentName);
	VehicleMovementComponent->SetIsReplicated(true); // Enable replication by default
	VehicleMovementComponent->UpdatedComponent = Mesh;
}

void AOffroadVehiclePawn::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	static FName NAME_Vehicle = FName(TEXT("Vehicle"));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
}

class UChaosVehicleMovementComponent* AOffroadVehiclePawn::GetVehicleMovementComponent() const
{
	return VehicleMovementComponent;
}

