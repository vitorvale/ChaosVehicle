// Code written by Vitor Vale


#include "OffroadWheeledVehicleSimulation.h"
#include "DrawDebugHelpers.h"
#include "DisplayDebugHelpers.h"
#include "DisplayDebugHelpers.h"
#include "Chaos/DebugDrawQueue.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#endif
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "VehicleAnimationInstance.h"
#include "ChaosVehicleManager.h"
#include "ChaosVehicleWheel.h"
#include "SuspensionUtility.h"
#include "SteeringUtility.h"
#include "TransmissionUtility.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Chaos/PBDSuspensionConstraintData.h"
#include "Chaos/DebugDrawQueue.h"
#include "UObject/UE5MainStreamObjectVersion.h"
#include "PhysicsProxy/SuspensionConstraintProxy.h"
#include "PBDRigidsSolver.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "SuspensionSystem.h"


using namespace Chaos;

OffroadWheeledVehicleSimulation::OffroadWheeledVehicleSimulation()
{
	FinalSuspensionPosition.Init(FVector::ZeroVector, 4);
	IntermediateSuspensionPosition.Init(FVector::ZeroVector, 4);
	TargetSuspensionPosition.Init(FVector::ZeroVector, 4);
	TotalInterpTime.Init(0.0f, 4);
	CurrentInterpTime.Init(0.0f, 4);
}


void OffroadWheeledVehicleSimulation::PerformSuspensionTraces(const TArray<Chaos::FSuspensionTrace>& SuspensionTrace, FCollisionQueryParams& TraceParams, FCollisionResponseContainer& CollisionResponse, TArray<FWheelTraceParams>& WheelTraceParams)
{

	ECollisionChannel SpringCollisionChannel = ECollisionChannel::ECC_WorldDynamic;
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse = CollisionResponse;

	for (int WheelIdx = 0; WheelIdx < SuspensionTrace.Num(); WheelIdx++)
	{
		FHitResult& HitResult = WheelState.TraceResult[WheelIdx];

		FVector TraceStart = SuspensionTrace[WheelIdx].Start;
		FVector TraceEnd = SuspensionTrace[WheelIdx].End;
		TraceParams.bTraceComplex = (WheelTraceParams[WheelIdx].SweepType == ESweepType::ComplexSweep);

		FVector TraceVector(TraceStart - TraceEnd); // reversed
		FVector TraceNormal = TraceVector.GetSafeNormal();

		switch (WheelTraceParams[WheelIdx].SweepShape)
		{
		case ESweepShape::Spherecast:
		{
			float WheelRadius = PVehicle->Wheels[WheelIdx].GetEffectiveRadius();
			FVector VehicleUpAxis = TraceNormal;

			SweepTypeSpherecast = true;
			
			bool bHit = World->SweepSingleByChannel(HitResult
				, TraceStart + VehicleUpAxis * WheelRadius
				, TraceEnd
				, FQuat::Identity, SpringCollisionChannel
				, FCollisionShape::MakeSphere(WheelRadius), TraceParams
				, ResponseParams);


			if (bHit)
			{
				float HeightDifference = (HitResult.ImpactPoint.Z + (WheelRadius * HitResult.ImpactNormal.Z)) - WheelState.WheelWorldLocation[WheelIdx].Z;

				if (abs(HeightDifference) > InterpHeightCutoff)
				{
					if (TotalInterpTime[WheelIdx] == 0) {
						float HeightRatio = FMath::Max(HitResult.ImpactPoint.Z, WheelState.WheelWorldLocation[WheelIdx].Z) / FMath::Min(HitResult.ImpactPoint.Z, WheelState.WheelWorldLocation[WheelIdx].Z);
						TotalInterpTime[WheelIdx] = FMath::Clamp(HeightRatio * InterpTimeHeightFactor, minInterpTime, maxInterpTime);
						FinalSuspensionPosition[WheelIdx] = HitResult.ImpactPoint + (WheelRadius * VehicleState.VehicleUpAxis);

						if (QuadraticInterpolation) 
						{
							
							IntermediateSuspensionPosition[WheelIdx] = WheelState.WheelWorldLocation[WheelIdx];
							IntermediateSuspensionPosition[WheelIdx].Z = HitResult.ImpactPoint.Z + (WheelRadius * VehicleState.VehicleUpAxis).Z;
						}

						UE_LOG(LogTemp, Warning, TEXT("Interp Height Difference Found: %f"), abs(HeightDifference));
					}
				}
				break;
			}

			// Piece of code that was going to be used to perform forward sweeps to predict future collisions and interpolate

			/*// Calculate the direction of the sweep
			float TraceDistance = 50.0f;
			FVector SweepDirection = PVehicle->GetWheel(WheelIdx).GroundVelocityVector;
			FVector EndPos = TraceStart + (SweepDirection * TraceDistance);

			// Calculate the step size for each sphere
			float NumSpheres = 64.0f;
			float StepSize = (EndPos - TraceStart).Size() / NumSpheres;

			for (int32 i = 0; i <= NumSpheres; ++i)
			{
				// Calculate the location for the current sphere
				FVector SphereCenter = TraceStart + SweepDirection * (StepSize * i);
				FVector EndTrace = TraceEnd + SweepDirection * (StepSize * i);

				// Perform a sphere trace at the current location

				bool bHit = World->SweepSingleByChannel(
					HitResult,
					SphereCenter,
					EndTrace, 
					FQuat::Identity,
					SpringCollisionChannel,
					FCollisionShape::MakeSphere(WheelRadius),
					TraceParams
				);
			}*/

			//UE_LOG(LogTemp, Warning, TEXT("Impact Point Location: %s"), *HitResult.ImpactPoint.ToString());
			//DrawDebugSphere(World, HitResult.ImpactPoint, 1.0f, 4.0f, FColor::Green, false, 1.0f, 0, 2.0f);

		}
		break;
		case ESweepShape::Raycast:
		default:
		{
			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, SpringCollisionChannel, TraceParams, ResponseParams);
			SweepTypeSpherecast = false;
			UE_LOG(LogTemp, Warning, TEXT("Raycast Sweep Shape!"));
		}
		break;
		}
	}

}

void OffroadWheeledVehicleSimulation::ApplySuspensionForces(float DeltaTime)
{
	float deltaStep = DeltaTime / 15.0f;
	TArray<float> SusForces;
	SusForces.Init(0.f, PVehicle->Suspension.Num());

	for (int WheelIdx = 0; WheelIdx < SusForces.Num(); WheelIdx++)
	{
		FHitResult& HitResult = WheelState.TraceResult[WheelIdx];
		FVector TargetPos = FVector();
		float NewDesiredLength = 1.0f; // suspension max length
		float ForceMagnitude2 = 0.f;
		auto& PWheel = PVehicle->Wheels[WheelIdx];
		auto& PSuspension = PVehicle->Suspension[WheelIdx];
		float SuspensionMovePosition = -PSuspension.Setup().MaxLength;
		
		if (TotalInterpTime[WheelIdx] > 0 && !QuadraticInterpolation)
		{
			CurrentInterpTime[WheelIdx] += DeltaTime;
			CurrentInterpTime[WheelIdx] = FMath::Clamp(CurrentInterpTime[WheelIdx], 0.0f, TotalInterpTime[WheelIdx]);

			float InterpolationFactor = FMath::SmoothStep(0.0f, 1.0f, CurrentInterpTime[WheelIdx] / TotalInterpTime[WheelIdx]);
			TargetPos = FMath::Lerp(WheelState.WheelWorldLocation[WheelIdx], FinalSuspensionPosition[WheelIdx], InterpolationFactor);
		}
		else if (TotalInterpTime[WheelIdx] > 0) 
		{
			CurrentInterpTime[WheelIdx] += DeltaTime;
			CurrentInterpTime[WheelIdx] = FMath::Clamp(CurrentInterpTime[WheelIdx], 0.0f, TotalInterpTime[WheelIdx]);

			// Calculate a quadratic interpolation factor
			float QuadraticInterpFactor = FMath::Square(CurrentInterpTime[WheelIdx] / TotalInterpTime[WheelIdx]);

			// Interpolate towards the new target position using quadratic interpolation
			TargetPos = FMath::Lerp(FMath::Lerp(WheelState.WheelWorldLocation[WheelIdx], IntermediateSuspensionPosition[WheelIdx], 1.0f - QuadraticInterpFactor),
				FMath::Lerp(IntermediateSuspensionPosition[WheelIdx], FinalSuspensionPosition[WheelIdx], 1.0f - QuadraticInterpFactor),
				QuadraticInterpFactor);
		}
		else {
			TargetPos = HitResult.ImpactPoint + (PWheel.GetEffectiveRadius() * VehicleState.VehicleUpAxis);
		}

		if (WheelIdx < ConstraintHandles.Num())
		{
			FPhysicsConstraintHandle& ConstraintHandle = ConstraintHandles[WheelIdx];
			if (ConstraintHandle.IsValid())
			{
				if (Chaos::FSuspensionConstraint* Constraint = static_cast<Chaos::FSuspensionConstraint*>(ConstraintHandle.Constraint))
				{
					if (FSuspensionConstraintPhysicsProxy* Proxy = Constraint->GetProxy<FSuspensionConstraintPhysicsProxy>())
					{

						Chaos::FPhysicsSolver* Solver = Proxy->GetSolver<Chaos::FPhysicsSolver>();

						Solver->SetSuspensionTarget(Constraint, TargetPos, HitResult.ImpactNormal, PWheel.InContact());
					}
				}
			}
		}


		if (PWheel.InContact())
		{
			NewDesiredLength = FVec3::Dist(TargetPos - HitResult.ImpactNormal * PWheel.GetEffectiveRadius(), WheelState.WheelWorldLocation[WheelIdx]);
			SuspensionMovePosition = -FVector::DotProduct(WheelState.WheelWorldLocation[WheelIdx] - (TargetPos - HitResult.ImpactNormal * PWheel.GetEffectiveRadius()), VehicleState.VehicleUpAxis) + PWheel.GetEffectiveRadius();

			if (!SweepTypeSpherecast) {
				NewDesiredLength = HitResult.Distance;
				SuspensionMovePosition = -FVector::DotProduct(WheelState.WheelWorldLocation[WheelIdx] - HitResult.ImpactPoint, VehicleState.VehicleUpAxis) + PWheel.GetEffectiveRadius();
				UE_LOG(LogTemp, Warning, TEXT("No interpolation!"));
			}

			PSuspension.SetSuspensionLength(NewDesiredLength, PWheel.GetEffectiveRadius());
			PSuspension.SetLocalVelocity(WheelState.LocalWheelVelocity[WheelIdx]);
			PSuspension.Simulate(DeltaTime);

			float ForceMagnitude = PSuspension.GetSuspensionForce();

			FVector GroundZVector = HitResult.Normal;
			FVector SuspensionForceVector = VehicleState.VehicleUpAxis * ForceMagnitude;

			FVector SusApplicationPoint = WheelState.WheelWorldLocation[WheelIdx] + PVehicle->Suspension[WheelIdx].Setup().SuspensionForceOffset;

			check(PWheel.InContact());

			ForceMagnitude = PSuspension.Setup().WheelLoadRatio * ForceMagnitude + (1.f - PSuspension.Setup().WheelLoadRatio) * PSuspension.Setup().RestingForce;
			PWheel.SetWheelLoadForce(ForceMagnitude);
			PWheel.SetMassPerWheel(RigidHandle->M() / PVehicle->Wheels.Num());
			SusForces[WheelIdx] = ForceMagnitude;

		}
		else
		{
			PSuspension.SetSuspensionLength(PSuspension.GetTraceLength(PWheel.GetEffectiveRadius()), PWheel.Setup().WheelRadius);
			PWheel.SetWheelLoadForce(0.f);

		}

		if (CurrentInterpTime[WheelIdx] >= TotalInterpTime[WheelIdx])
		{
			CurrentInterpTime[WheelIdx] = 0.0f;
			TotalInterpTime[WheelIdx] = 0.0f;
		}
	}


	for (auto& Axle : PVehicle->GetAxles())
	{
		if (Axle.Setup.WheelIndex.Num() == 2)
		{
			uint16 WheelIdxA = Axle.Setup.WheelIndex[0];
			uint16 WheelIdxB = Axle.Setup.WheelIndex[1];

			float FV = Axle.Setup.RollbarScaling;
			float ForceDiffOnAxleF = SusForces[WheelIdxA] - SusForces[WheelIdxB];
			FVector ForceVector0 = VehicleState.VehicleUpAxis * ForceDiffOnAxleF * FV;
			FVector ForceVector1 = VehicleState.VehicleUpAxis * ForceDiffOnAxleF * -FV;

			FVector SusApplicationPoint0 = WheelState.WheelWorldLocation[WheelIdxA] + PVehicle->Suspension[WheelIdxA].Setup().SuspensionForceOffset;
			AddForceAtPosition(ForceVector0, SusApplicationPoint0);

			FVector SusApplicationPoint1 = WheelState.WheelWorldLocation[WheelIdxB] + PVehicle->Suspension[WheelIdxB].Setup().SuspensionForceOffset;
			AddForceAtPosition(ForceVector1, SusApplicationPoint1);
		}
	}
}


