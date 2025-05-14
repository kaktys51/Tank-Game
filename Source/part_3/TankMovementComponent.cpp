
#include "TankMovementComponent.h"
#include "TankPawn.h"

UTankMovementComponent::UTankMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	PawnOwner = GetPawnOwner();
	TankOwner = Cast<ATankPawn>(PawnOwner);
	FTransform OwnerTransform = TankOwner->GetTransform();

	if (TankOwner->HasAuthority())
	{
		SimProxyTransform = OwnerTransform;
	}

	// Visual smoothing setup
	BodyVisualLocation = OwnerTransform.GetLocation();
	BodyVisualRotation = OwnerTransform.GetRotation();

	bIsUpdateVisualActive = true;

	// Adding line trace starting points from owner class
	if (TankOwner)
	{
		Samples = { TankOwner->GroundSampleFL,
					TankOwner->GroundSampleFR,
					TankOwner->GroundSampleBL,
					TankOwner->GroundSampleBR
		};
	}
}

void UTankMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent)
	{
		return;
	}
	
	// Applies gravitational pull and align tank to surface
	SimpleGravity(DeltaTime);

	if (TankOwner->HasAuthority() && !UpdatedComponent->GetComponentTransform().Equals(SimProxyTransform))
	{
		SimProxyTransform = UpdatedComponent->GetComponentTransform();
	}

	// Simulated proxy movement
	if (bIsSimProxyTransformUpdated)
	{
		SimulatedProxyMovement(DeltaTime);
	}

	// Prepearing move data to send to server
	FTankSafeMove NewMove;

	NewMove.TimeStamp = GetWorld()->GetTimeSeconds();
	NewMove.DeltaTime = DeltaTime;

	// Moving forward/backward logic
	if (!PendingInput.IsNearlyZero())
	{
		bIsPreformedMove = true;

		// Calculate movement distance 
		FVector DesiredMovement = PendingInput.GetClampedToMaxSize(1.0f) * MoveSpeed * DeltaTime;

		// Preforming movement
		FHitResult Hit;
		SafeMoveUpdatedComponent(DesiredMovement, UpdatedComponent->GetComponentRotation(), true, Hit);

		//FVector AfterLoc = TankOwner->GetActorLocation();
		//UE_LOG(LogTemp, Warning, TEXT("After: %f, %f, %f"), AfterLoc.X, AfterLoc.Y, AfterLoc.Z);

		// Slide if hit collitsion
		if (Hit.IsValidBlockingHit())
		{
			SlideAlongSurface(DesiredMovement, 1.0f - Hit.Time, Hit.Normal, Hit);
		}

		// Creating and saving move data
		NewMove.InputVector = PendingInput;
		NewMove.SavedLocation = TankOwner->GetActorLocation();
		NewMove.SavedRotation = TankOwner->GetActorRotation();

	}

	// Turning hull of the tank
	if (FMath::Abs(PendingTurnInput) > DeadZoneTurning)
	{
		bIsPreformedTurn = true;

		Turn(PendingTurnInput);

		NewMove.InputRotation = PendingTurnInput;
		NewMove.SavedRotation = TankOwner->GetActorRotation();
		NewMove.SavedLocation = TankOwner->GetActorLocation();

	}

	// Checking if pawn is locally controlled AND if any movement or turning was made
	if (PawnOwner->IsLocallyControlled() && !PawnOwner->HasAuthority() && (bIsPreformedMove || bIsPreformedTurn))
	{
		SavedMoves.Add(NewMove);
		//Sending moving data to server
		Server_Move(NewMove);
	}

	// Visual interpolation of tank hull
	if (bIsUpdateVisualActive)
	{
		UpdateVisual(DeltaTime);
	}

	 //  ********
	 //**** IMPORATNT !!!!! ****
	 // after preformed movement\turning clears pending values
	 //  ********
	PendingInput = FVector::ZeroVector;
	PendingTurnInput = 0.f;
	bIsPreformedMove = false;
	bIsPreformedTurn = false;
}

void UTankMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UTankMovementComponent, SimProxyTransform, COND_SimulatedOnly);
}

void UTankMovementComponent::UpdateVisual(float DeltaTime)
{
	if (!TankOwner) return;
	
	if (bIsCorrectionActive)
	{
		//Smoothe interpolation after correction from VisualRoot to UpdateComponent (actor Root)

		float AdaptiveInterpDuration = CorrectInterpDuration;

		// Prevent recalculating of AdaptiveInterpDuration during continues interpolation. Calculates only on first tick of correction
		if (!bIsCoorectionOngoing)
		{
			const FVector2D InRange = FVector2D(DistanceThresholdLow, DistanceThresholdHigh);
			const FVector2D OutRange = FVector2D(MaxDuration, MinDuration);

			AdaptiveInterpDuration = FMath::GetMappedRangeValueClamped(InRange, OutRange, CurrentCorrectionDistance);

			bIsCoorectionOngoing = true;
		}

		CorrectInterpTimeElapsded += DeltaTime;
		//float Alpha = FMath::Clamp(CorrectInterpTimeElapsded / CorrectInterpDuration, 0.f, 1.f);
		float Alpha = FMath::Clamp(CorrectInterpTimeElapsded / AdaptiveInterpDuration, 0.f, 1.f);

		FVector OwnerLocation = TankOwner->GetActorLocation();
		FQuat OwnerRotation = TankOwner->GetActorRotation().Quaternion();

		FVector NewLocation = FMath::Lerp(BodyVisualLocation, OwnerLocation, FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, CrorrectAphaCurveture));
		FQuat NewRotation = FQuat::Slerp(BodyVisualRotation, OwnerRotation, Alpha);

		TankOwner->TankVisualRoot->SetWorldLocation(NewLocation);
		TankOwner->TankVisualRoot->SetWorldRotation(NewRotation);

		if (Alpha >= 1.f)
		{
			bIsCorrectionActive = false;
			bIsCoorectionOngoing = false;
			CorrectInterpTimeElapsded = 0.f;
		}
	}
	else
	{
		// If correction interpolation not active, just folowing UpdateComponent

		FTransform OwnerTransform = TankOwner->GetTransform();

		BodyVisualLocation = OwnerTransform.GetLocation();
		BodyVisualRotation = OwnerTransform.GetRotation();
		//BodyVisualRotation = AlignVisualRootToGround();

		TankOwner->TankVisualRoot->SetWorldLocation(BodyVisualLocation);
		TankOwner->TankVisualRoot->SetWorldRotation(BodyVisualRotation);

	}
}

void UTankMovementComponent::OnRep_SimProxyTransform()
{
	bIsSimProxyTransformUpdated = true;
}

void UTankMovementComponent::SimulatedProxyMovement(float DeltaTime)
{
	if (!TankOwner->HasAuthority() && !TankOwner->IsLocallyControlled())
	{
		FTransform CurrentTransform = UpdatedComponent->GetComponentTransform();
		FVector CurrentLocation = CurrentTransform.GetLocation();
		FRotator CurrentRotation = CurrentTransform.GetRotation().Rotator();

		FVector NewLocation = FMath::VInterpTo(CurrentLocation, SimProxyTransform.GetLocation(), DeltaTime, SimProxyAlpha);
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, SimProxyTransform.GetRotation().Rotator(), DeltaTime, SimProxyAlpha);
		FTransform NewTransform(NewRotation, NewLocation, CurrentTransform.GetScale3D());

		UpdatedComponent->SetWorldTransform(NewTransform);
		
		bool bLocationEqual = CurrentLocation.Equals(NewLocation, 0.01f);
		bool bRotationEqual = CurrentRotation.Equals(NewRotation, 0.01f);

		// If actor performed necessary movement and rotation, disables calling for this function
		if (bLocationEqual && bRotationEqual)
		{
			bIsSimProxyTransformUpdated = false;
		}
	}
}

void UTankMovementComponent::AddMoveInputVector(const FVector& WorldVector)
{
	PendingInput += WorldVector;
}

void UTankMovementComponent::AddTurnValue(const float Direction)
{
	PendingTurnInput = Direction;
}

void UTankMovementComponent::Server_Move_Implementation(const FTankSafeMove& MoveData)
{
	if (MoveData.TimeStamp <= LastProcessedMoveTimeStamp) return;

	LastProcessedMoveTimeStamp = MoveData.TimeStamp;

	UE_LOG(LogTemp, Warning, TEXT("Server exec"));
	if (!MoveData.InputVector.IsNearlyZero())
	{
		FVector DesiredMovement = MoveData.InputVector.GetClampedToMaxSize(1.0f) * MoveSpeed * MoveData.DeltaTime;
		FHitResult Hit;
		SafeMoveUpdatedComponent(DesiredMovement, UpdatedComponent->GetComponentRotation(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			SlideAlongSurface(DesiredMovement, 1.0f - Hit.Time, Hit.Normal, Hit);
		}
	}

	if (FMath::Abs(MoveData.InputRotation) > DeadZoneTurning)
	{
		Turn(MoveData.InputRotation);
	}

	// Checking for error after moving, and send correction if needed
	CheckMovementError(MoveData);
}

void UTankMovementComponent::CheckMovementError(const FTankSafeMove& MoveData)
{
	// Checking for distance error, if not found, checking for rotation error
	FVector ServerLocation = PawnOwner->GetActorLocation();
	FVector ClientLocation = MoveData.SavedLocation;

	// Cheking distance betven server calculation and client data
	float DistanceError = FVector::DistSquared(ServerLocation, ClientLocation);
	float CorrectionValue = FMath::Square(DistanceCorrection);

	UE_LOG(LogTemp, Warning, TEXT("DistanceError: %f"), DistanceError);

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (DistanceError >= CorrectionValue && (CurrentTime - LastCorrectionTime) > CorrectionCooldown)
	{
		// Sending corrections to client

		LastCorrectionTime = CurrentTime;
		Multicast_CorrectionData(UpdatedComponent->GetComponentTransform(), MoveData.TimeStamp);
		return;
	}

	// If distance was in error tolerance, checking for rotation error
	FRotator ServerRotation = PawnOwner->GetActorRotation();
	FRotator ClientRotation = MoveData.SavedRotation;

	float YawDifference = FMath::FindDeltaAngleDegrees(ServerRotation.Yaw, ClientRotation.Yaw);
	float PitchDifference = FMath::FindDeltaAngleDegrees(ServerRotation.Pitch, ClientRotation.Pitch);
	float RollDifference = FMath::FindDeltaAngleDegrees(ServerRotation.Roll, ClientRotation.Roll);

	float AbsYawDiff = FMath::Abs(YawDifference);
	float AbsPitchDiff = FMath::Abs(PitchDifference);
	float AbsRollDiff = FMath::Abs(RollDifference);

	if (AbsYawDiff > TurnCorrection || AbsPitchDiff > TurnCorrection || AbsRollDiff > TurnCorrection && (CurrentTime - LastCorrectionTime) > CorrectionCooldown)
	{
		// Sending corrections to client

		LastCorrectionTime = CurrentTime;
		Multicast_CorrectionData(UpdatedComponent->GetComponentTransform(), MoveData.TimeStamp);
		return;
	}
}

void UTankMovementComponent::Multicast_CorrectionData_Implementation(const FTransform& ServerTransform, float TimeStamp)
{
	if (!TankOwner->HasAuthority())
	{
		FVector CurrentLocation = TankOwner->GetActorLocation();
		FVector ServerLocation = ServerTransform.GetLocation();

		CurrentCorrectionDistance = FVector::Dist(CurrentLocation, ServerLocation);

		UpdatedComponent->SetWorldTransform(ServerTransform);

		TArray<FTankSafeMove> MovesToReplay;
		for (const FTankSafeMove& Move : SavedMoves)
		{
			if (Move.TimeStamp > TimeStamp)
			{
				MovesToReplay.Add(Move);
			}
		}
		SavedMoves = MovesToReplay;

		for (const FTankSafeMove& Move : SavedMoves)
		{
			FVector DesiredMovement = Move.InputVector.GetClampedToMaxSize(1.0f) * MoveSpeed * Move.DeltaTime;
			FHitResult Hit;
			SafeMoveUpdatedComponent(DesiredMovement, UpdatedComponent->GetComponentRotation(), true, Hit);
			if (Hit.IsValidBlockingHit())
			{
				SlideAlongSurface(DesiredMovement, 1.0f - Hit.Time, Hit.Normal, Hit);
			}

			Turn(Move.InputRotation);
		}

		bIsCorrectionActive = true;

	}
}

void UTankMovementComponent::Turn(float Amount)
{
	//CurrentTurnAmount = FMath::FInterpTo(CurrentTurnAmount, Amount, GetWorld()->GetDeltaSeconds(), RotationAccelerationDuration);

	float DeltaRotation = Amount * TurnSpeed * GetWorld()->GetDeltaSeconds();
	FRotator Rotation = FRotator(0.f, DeltaRotation, 0.f);

	//FRotator Rotation = FRotator(0.f, CurrentTurnAmount, 0.f);
	TankOwner->AddActorLocalRotation(Rotation);
	
}

FQuat UTankMovementComponent::AlignVisualRootToGround()
{
	if (!TankOwner || !TankOwner->TankVisualRoot) return FQuat::Identity;

	if (Samples.Num() < 4) return TankOwner->GetActorQuat();

	TArray<FVector> Normals;
	Normals.Reserve(Samples.Num());

	for (USceneComponent* Sample : Samples)
	{
		if (!Sample) continue;

		FVector Start = Sample->GetComponentLocation();
		FVector End = Start - FVector::UpVector * TraceDepth; // вниз на TraceDepth

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(PawnOwner);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			Normals.Add(Hit.ImpactNormal);
		}
	}

	FVector AvgNormal = FVector::UpVector;
	if (Normals.Num() > 0)
	{
		AvgNormal = FVector::ZeroVector;
		for (const FVector& N : Normals)
		{
			AvgNormal += N;
		}
		AvgNormal = AvgNormal.GetSafeNormal();
	}

	const FVector Forward = PawnOwner->GetActorForwardVector();
	const FVector Right = FVector::CrossProduct(AvgNormal, Forward).GetSafeNormal();
	const FVector SurfaceForward = FVector::CrossProduct(Right, AvgNormal).GetSafeNormal();

	FMatrix RotationMatrix(SurfaceForward, Right, AvgNormal, FVector::ZeroVector);
	return FQuat(RotationMatrix);
}

void UTankMovementComponent::SimpleGravity(float DeltaTime)
{
	const float GravityStrength = 980.f; // как в UE по умолчанию
	FVector GravityVector = FVector(0, 0, -1) * GravityStrength * DeltaTime;

	float TraceLength = TankOwner->CapsuleComponent->GetScaledCapsuleHalfHeight() + 15.f;

	FVector Start = TankOwner->CapsuleComponent->GetComponentLocation();
	FVector End = Start - FVector::UpVector * TraceLength;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(TankOwner);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		//AlignVisualRootToGround();

		FQuat ActorRotation = TankOwner->CapsuleComponent->GetComponentRotation().Quaternion();
		FQuat TargetRotation = AlignVisualRootToGround();
		FQuat NewRotation = FQuat::Slerp(ActorRotation, TargetRotation, DeltaTime * AlligmentSpeed);

		TankOwner->CapsuleComponent->SetWorldRotation(NewRotation);
	}
	else
	{
		// Apply gravity pull 
		UpdatedComponent->AddWorldOffset(GravityVector, true);

		FQuat ActorRotation = TankOwner->CapsuleComponent->GetComponentRotation().Quaternion();
		FQuat TargetRotation = AlignVisualRootToGround();
		FQuat NewRotation = FQuat::Slerp(ActorRotation, TargetRotation, DeltaTime * AlligmentSpeed);

		TankOwner->CapsuleComponent->SetWorldRotation(NewRotation);
		//FHitResult Hit;
		//MoveComponent(GravityVector, UpdatedComponent->GetComponentQuat(), true, &Hit);
	}
}