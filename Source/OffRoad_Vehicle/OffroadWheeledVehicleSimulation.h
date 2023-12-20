// Code written by Vitor Vale

#pragma once

#include "ChaosWheeledVehicleMovementComponent.h"
#include "CoreMinimal.h"

/**
 * 
 */
class OFFROAD_VEHICLE_API OffroadWheeledVehicleSimulation : public UChaosWheeledVehicleSimulation
{

public:

	OffroadWheeledVehicleSimulation();

	void PerformSuspensionTraces(const TArray<Chaos::FSuspensionTrace>& SuspensionTrace, FCollisionQueryParams& TraceParams, FCollisionResponseContainer& CollisionResponse, TArray<FWheelTraceParams>& WheelTraceParams) override;



	void ApplySuspensionForces(float DeltaTime) override;


	TArray <float> TotalInterpTime;              // Total time over which the interpolation will occur
	TArray <float> CurrentInterpTime;            // Current elapsed time during the interpolation
	float maxInterpTime = 0.25f;				 // Maximum time interval that can be used for interpolation
	float minInterpTime = 0.0001f;				 // Minimum time interval that can be used for interpolation
	float InterpHeightCutoff = 26.0f;			 // Height at which the interpolation is performed to smooth movement
	float InterpTimeHeightFactor = 0.1f;		 // Factor multiplied by height ratio to obtain interpolation time	
	bool QuadraticInterpolation = true;			 // Parameter to change between Quadratic and Linear Interpolation
	bool SweepTypeSpherecast = false;

	TArray <FVector> TargetSuspensionPosition;			// Vector Array that stores interpolated suspension target position for each wheel
	TArray <FVector> FinalSuspensionPosition;			// Vector Array that stores final suspension position position for each wheel
	TArray <FVector> IntermediateSuspensionPosition;	// Vector Array that stores intermediate suspension position for each wheel (only used in Quadratic Interpolation)


};
