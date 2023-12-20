// Code written by Vitor Vale

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OffroadVehiclePawn.generated.h"

UCLASS()
class OFFROAD_VEHICLE_API AOffroadVehiclePawn : public APawn
{
	GENERATED_UCLASS_BODY()

private:
	/**  The main skeletal mesh associated with this Vehicle */
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh;

	/** vehicle simulation component */
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UChaosVehicleMovementComponent> VehicleMovementComponent;

public:	

	/** Name of the MeshComponent. Use this name if you want to prevent creation of the component (with ObjectInitializer.DoNotCreateDefaultSubobject). */
	static FName VehicleMeshComponentName;

	/** Name of the VehicleMovement. Use this name if you want to use a different class (with ObjectInitializer.SetDefaultSubobjectClass). */
	static FName VehicleMovementComponentName;

	/** Util to get the wheeled vehicle movement component */
	class UChaosVehicleMovementComponent* GetVehicleMovementComponent() const;

	//~ Begin AActor Interface
	virtual void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	//~ End Actor Interface

	/** Returns Mesh subobject **/
	class USkeletalMeshComponent* GetMesh() const { return Mesh; }

	/** Returns VehicleMovement subobject **/
	class UChaosVehicleMovementComponent* GetVehicleMovement() const { return VehicleMovementComponent; }

};
