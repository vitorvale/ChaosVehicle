// Code written by Vitor Vale


#include "Offroad_WheeledVehicle_Pawn.h"
#include "DrawDebugHelpers.h"
#include "ChaosVehicleMovementComponent.h"
#include <Engine/StaticMeshActor.h>
#include <Kismet/GameplayStatics.h>

AOffroad_WheeledVehicle_Pawn::AOffroad_WheeledVehicle_Pawn()
{
    // Setting this pawn to call Tick() every frame
    AActor::PrimaryActorTick.bCanEverTick = true;

    // Creating the static mesh component
    AttachedCube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedCube"));

    // Setting the cube mesh to the default cube
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
    if (CubeMeshAsset.Succeeded())
    {
        AttachedCube->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Attaching the cube to the root component
    AttachedCube->SetupAttachment(RootComponent);

    // Creating the physics constraint component
    CubeConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("CubeConstraint"));
    CubeConstraint->SetupAttachment(RootComponent);

    // Setting a reference for the mesh on the VehiclePart struct
    OffroadVehiclePart.CubeMesh = AttachedCube;

    GetMesh()->OnComponentHit.AddDynamic(this, &AOffroad_WheeledVehicle_Pawn::OnHit);
    AttachedCube->OnComponentHit.AddDynamic(this, &AOffroad_WheeledVehicle_Pawn::OnHit);
}

void AOffroad_WheeledVehicle_Pawn::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> FoundActors;

    // Attach the cube to the vehicle mesh
    if (GetMesh())
    {
        CubeConstraint->SetConstrainedComponents(GetMesh(), NAME_None, AttachedCube, NAME_None);
    }

    UClass* TriggerVolumeClass = ATriggerVolume::StaticClass();

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), TriggerVolumeClass, FoundActors);

    if (FoundActors.Num() > 0)
    {
        // Get the first actor found
        AActor* FirstActor = FoundActors[0];

        // Cast the actor to ATriggerVolume
        RepairTriggerVolume = Cast<ATriggerVolume>(FirstActor);

    }
}

void AOffroad_WheeledVehicle_Pawn::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    UE_LOG(LogTemp, Warning, TEXT("Component Hit: %s !"), *HitComponent->GetName());
    UE_LOG(LogTemp, Warning, TEXT("Other Actor Hit: %s !"), *OtherActor->GetName());

    float ImpactIntensity;
    // Calculate impact intensity based on the normal impulse and mass of the car
    UChaosVehicleMovementComponent* OffroadVehicleMovementComponent = GetVehicleMovementComponent();
    if (OffroadVehicleMovementComponent) {
        if (OtherComp->GetOwner() != HitComponent->GetOwner()) {
            float Mass = OffroadVehicleMovementComponent->Mass;

            UE_LOG(LogTemp, Warning, TEXT("Mass of Car: %f"), Mass);

            ImpactIntensity = NormalImpulse.Size() / Mass;

            UE_LOG(LogTemp, Warning, TEXT("Impact Intensity: %f"), ImpactIntensity);

            // Apply damage to vehicle part using the impact intensity
            if (OffroadVehiclePart.CurrentDurability > 0) {
                ApplyDamageToVehiclePart(ImpactIntensity, HitComponent);
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("Vehicle part is not attached!"));
            }
            
        } 
        else{
            UE_LOG(LogTemp, Warning, TEXT("Self Impact ignored!"));
        }
    }

}

void AOffroad_WheeledVehicle_Pawn::ApplyDamageToVehiclePart(float ImpactIntensity, UPrimitiveComponent* HitComponent)
{
    float DamageScaleFactor;
    float DamageAmount;
    float NewDurability;

    if (HitComponent == AttachedCube) {
        DamageScaleFactor = 1;
    }
    else {
        DamageScaleFactor = 0.01f;
    }
 
    DamageAmount = ImpactIntensity * DamageScaleFactor;
    NewDurability = OffroadVehiclePart.CurrentDurability - DamageAmount;
    UE_LOG(LogTemp, Warning, TEXT("New Durability: %f"), NewDurability);

    if (NewDurability <= 0) {
        //Update durability and detach the VehiclePart
        OffroadVehiclePart.CurrentDurability = 0;
        DetachVehiclePart();
    }
    else {
        //Apply the damage to the VehiclePart
        OffroadVehiclePart.CurrentDurability = NewDurability;
    }
}

void AOffroad_WheeledVehicle_Pawn::DetachVehiclePart()
{
    //VehiclePart is detached from off road vehicle
    CubeConstraint->BreakConstraint();
    AttachedCube->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);


    // Get the collision response of the detached mesh
    FCollisionResponseContainer CollisionResponse = AttachedCube->GetCollisionResponseToChannels();

    // Set collision response for the vehicle and world dynamic channel
    CollisionResponse.SetResponse(ECC_Vehicle, ECR_Ignore);
    CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Ignore);

    // Apply the updated collision response to the detached mesh
    AttachedCube->SetCollisionResponseToChannels(CollisionResponse);

    AttachedCube->SetNotifyRigidBodyCollision(false);

    // Enable physics simulation for the detached cube mesh
    AttachedCube->SetSimulatePhysics(true);
}

void AOffroad_WheeledVehicle_Pawn::OnTriggerOverlap(AActor* ActorOverlaped, AActor* OtherActor)
{
    UE_LOG(LogTemp, Warning, TEXT("Trigger Overlaped!"));
    if (this && AttachedCube) {
        if ((OffroadVehiclePart.CurrentDurability == 0))
        {
            // Reattach the detached mesh to the vehicle
            if (AttachVehiclePart()) {
                // Reset the state of the Vehicle Part to MaxDurability
                OffroadVehiclePart.CurrentDurability = OffroadVehiclePart.MaxDurability;
                UE_LOG(LogTemp, Warning, TEXT("Cube Successfully Attached!"));
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("Cube Failed to Attach!"));
            }
        }
        else {
            UE_LOG(LogTemp, Warning, TEXT("Cube already attached!"));
        }
    }
    
}

bool AOffroad_WheeledVehicle_Pawn::AttachVehiclePart()
{
    bool AttachResult;

    AttachedCube->SetSimulatePhysics(false);

    AttachResult = AttachedCube->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);

    // Reset the state of the detached mesh
    AttachedCube->SetRelativeLocation(AttachmentRelativeLocation);
    AttachedCube->SetRelativeRotation(FRotator::ZeroRotator);
    AttachedCube->SetRelativeScale3D(AttachmentRelativeScale);
    AttachedCube->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Get the collision response of the mesh
    FCollisionResponseContainer CollisionResponse = AttachedCube->GetCollisionResponseToChannels();

    // Set collision response for the vehicle channel
    CollisionResponse.SetResponse(ECC_Vehicle, ECR_Block);
    CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Block);

    // Apply the updated collision response to the mesh
    AttachedCube->SetCollisionResponseToChannels(CollisionResponse);

    // Reapply the constraint
    if (CubeConstraint)
    {
        // Adjusting constraint setup parameters
        CubeConstraint->ConstraintInstance.SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
        CubeConstraint->ConstraintInstance.SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
        CubeConstraint->ConstraintInstance.SetAngularTwistMotion(EAngularConstraintMotion::ACM_Locked);
        CubeConstraint->ConstraintInstance.SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
        CubeConstraint->ConstraintInstance.SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
        CubeConstraint->ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);

        // Initiate the constraint with the current configuration
        CubeConstraint->InitComponentConstraint();
    }
    AttachedCube->SetNotifyRigidBodyCollision(true);
    AttachedCube->SetSimulatePhysics(true);

    return AttachResult;
}

// Another type of calculations for impact intensity (effectively not in use at the moment)
void AOffroad_WheeledVehicle_Pawn::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    UE_LOG(LogTemp, Warning, TEXT("Hit something!"));

    float ImpactIntensity;
    // Calculate impact intensity based on the normal impulse and other factors
    UChaosVehicleMovementComponent* OffroadVehicleMovementComponent = GetVehicleMovementComponent();
    if (OffroadVehicleMovementComponent) {
        FVector RelativeVelocity = OffroadVehicleMovementComponent->GetOwner()->GetVelocity() - OtherActor->GetVelocity();
        float Mass1 = OffroadVehicleMovementComponent->Mass;
        float Mass2 = OtherActor->FindComponentByClass<UPrimitiveComponent>()->GetMass(); // Assumes the other actor has a primitive component

        ImpactIntensity = RelativeVelocity.Size() * (Mass1 * Mass2) / (Mass1 + Mass2);

        // Apply damage to VehiclePart

        UE_LOG(LogTemp, Warning, TEXT("Impact Intensity: %f"), ImpactIntensity);
    }
}

