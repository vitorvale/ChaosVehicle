// Code written by Vitor Vale

#pragma once

#include <PhysicsEngine/PhysicsConstraintComponent.h>
#include <Engine/TriggerVolume.h>
#include "CoreMinimal.h"
#include "VehiclePart.h"
#include "WheeledVehiclePawn.h"
#include "OffroadVehiclePawn.h"
#include "Offroad_WheeledVehicle_Pawn.generated.h"


UCLASS()
class OFFROAD_VEHICLE_API AOffroad_WheeledVehicle_Pawn : public AOffroadVehiclePawn
{
	GENERATED_BODY()

	protected:
		virtual void BeginPlay() override;

		UFUNCTION(BlueprintCallable)
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

		UFUNCTION(BlueprintCallable)
        void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

		UFUNCTION()
		void ApplyDamageToVehiclePart(float ImpactIntensity, UPrimitiveComponent* HitComponent);

		UFUNCTION()
		void DetachVehiclePart();

		UFUNCTION()
		bool AttachVehiclePart();

		UFUNCTION(BlueprintCallable)
		void OnTriggerOverlap(AActor* ActorOverlaped, AActor* OtherActor);

	public:

		AOffroad_WheeledVehicle_Pawn();

		UPROPERTY(BlueprintReadWrite)
		ATriggerVolume* RepairTriggerVolume;


	private:
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* AttachedCube;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UPhysicsConstraintComponent* CubeConstraint;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		FVehiclePart OffroadVehiclePart;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		FVector AttachmentRelativeLocation = FVector(-199.432661f, 0.373213f, 110.712466f);

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		FVector AttachmentRelativeScale = FVector(0.5f, 0.5f, 0.5f);

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		FName SocketName = "CubeMeshSocket";

};
