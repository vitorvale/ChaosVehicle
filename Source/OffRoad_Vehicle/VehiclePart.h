// Code written by Vitor Vale

#pragma once

#include "CoreMinimal.h"
#include "VehiclePart.generated.h"

USTRUCT(BlueprintType)
struct OFFROAD_VEHICLE_API FVehiclePart
{

	GENERATED_USTRUCT_BODY()

public:
    FVehiclePart()
        : MaxDurability(100.0f), CurrentDurability(100.0f)
    {
    }

	// Reference to the cube mesh
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VehiclePart")
    UStaticMeshComponent* CubeMesh;

    // Maximum durability of the vehicle part
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VehiclePart")
    float MaxDurability;

    // Current durability of the vehicle part
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VehiclePart")
    float CurrentDurability;
};
