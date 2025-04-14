
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
}

void UTankMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent)
	{
		return;
	}
	
	if (TankOwner->HasAuthority() && !UpdatedComponent->GetComponentTransform().Equals(SimProxyTransform))
	{
		SimProxyTransform = UpdatedComponent->GetComponentTransform();
	}

	// Location replication for simulated proxy 
	if (!TankOwner->HasAuthority() && !TankOwner->IsLocallyControlled())
	{
		FVector proxyLoc = SimProxyTransform.GetLocation();

		//UE_LOG(LogTemp, Warning, TEXT("SimProxyTransform: %f, %f, %f"), proxyLoc.X, proxyLoc.Y, proxyLoc.Z);
		if (bIsSimProxyTransformUpdated)
		{
			FTransform CurrentTransform = UpdatedComponent->GetComponentTransform();

			FVector NewLocation = FMath::VInterpTo(CurrentTransform.GetLocation(), SimProxyTransform.GetLocation(), GetWorld()->GetDeltaSeconds(), SimProxyAlpha);
			FRotator NewRotation = FMath::RInterpTo(CurrentTransform.GetRotation().Rotator(), SimProxyTransform.GetRotation().Rotator(), GetWorld()->GetDeltaSeconds(), SimProxyAlpha);
			FTransform NewTransform(NewRotation, NewLocation, CurrentTransform.GetScale3D());

			UpdatedComponent->SetWorldTransform(NewTransform);
		}
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

	if (bIsVisualCorrectionActive)
	{
		VisualCorrectionSmooting(DeltaTime);
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

		CorrectInterpTimeElapsded += DeltaTime;
		float Alpha = FMath::Clamp(CorrectInterpTimeElapsded / CorrectInterpDuration, 0.f, 1.f);

		FVector OwnerLocation = TankOwner->GetActorLocation();
		FQuat OwnerRotation = TankOwner->GetActorRotation().Quaternion();

		FVector NewLocation = FMath::Lerp(BodyVisualLocation, OwnerLocation, FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, CrorrectAphaCurveture));
		FQuat NewRotation = FQuat::Slerp(BodyVisualRotation, OwnerRotation, Alpha);

		TankOwner->TankVisualRoot->SetWorldLocation(NewLocation);
		TankOwner->TankVisualRoot->SetWorldRotation(NewRotation);

		TankOwner->SmoothBoxTest->SetWorldLocation(NewLocation);

		if (Alpha >= 1.f)
		{
			bIsCorrectionActive = false;
			CorrectInterpTimeElapsded = 0.f;
		}
	}
	else
	{
		// If correction interpolation not active, just folowing UpdateComponent

		FTransform OwnerTransform = TankOwner->GetTransform();

		BodyVisualLocation = OwnerTransform.GetLocation();
		BodyVisualRotation = OwnerTransform.GetRotation();

		TankOwner->SmoothBoxTest->SetWorldLocation(OwnerTransform.GetLocation());

		TankOwner->TankVisualRoot->SetWorldLocation(BodyVisualLocation);
		TankOwner->TankVisualRoot->SetWorldRotation(BodyVisualRotation);

	}
}

void UTankMovementComponent::OnRep_SimProxyTransform()
{
	bIsSimProxyTransformUpdated = true;
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
	// добавить проверку не сортированных движений 
	// добавить велосити  ипроверку через расчет велосити ?

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
		LastCorrectionTime = CurrentTime;
		Multicast_CorrectionData(UpdatedComponent->GetComponentTransform(), MoveData.TimeStamp);
		return;
	}

	// If distance was in error tolerance, checking for rotation error
	FRotator ServerRotation = PawnOwner->GetActorRotation();
	FRotator ClientRotation = MoveData.SavedRotation;

	//FRotator DeltaRotation = ServerRotation - ClientRotation;

	//float DeltaYaw = FMath::Abs(DeltaRotation.Yaw);
	//float DeltaPitch = FMath::Abs(DeltaRotation.Pitch);
	//float DeltaRoll = FMath::Abs(DeltaRotation.Roll);

	float YawDifference = FMath::FindDeltaAngleDegrees(ServerRotation.Yaw, ClientRotation.Yaw);
	float PitchDifference = FMath::FindDeltaAngleDegrees(ServerRotation.Pitch, ClientRotation.Pitch);
	float RollDifference = FMath::FindDeltaAngleDegrees(ServerRotation.Roll, ClientRotation.Roll);

	float AbsYawDiff = FMath::Abs(YawDifference);
	float AbsPitchDiff = FMath::Abs(PitchDifference);
	float AbsRollDiff = FMath::Abs(RollDifference);

	if (AbsYawDiff > TurnCorrection || AbsPitchDiff > TurnCorrection || AbsRollDiff > TurnCorrection && (CurrentTime - LastCorrectionTime) > CorrectionCooldown)
	{
		LastCorrectionTime = CurrentTime;
			Multicast_CorrectionData(UpdatedComponent->GetComponentTransform(), MoveData.TimeStamp);
			return;
	}
	//if (DeltaYaw > TurnCorrection || DeltaPitch > TurnCorrection || DeltaRoll > TurnCorrection && (CurrentTime - LastCorrectionTime) > CorrectionCooldown)
	//{
	//	LastCorrectionTime = CurrentTime;
	//	Multicast_CorrectionData(UpdatedComponent->GetComponentTransform(), MoveData.TimeStamp);
	//	return;
	//}
}

void UTankMovementComponent::Multicast_CorrectionData_Implementation(const FTransform& ServerTransform, float TimeStamp)
{
	if (!TankOwner->HasAuthority())
	{
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
		}

		bIsCorrectionActive = true;

	}
}

void UTankMovementComponent::VisualCorrectionSmooting(float DeltaTime)
{
	if (!TankOwner) return;

	UStaticMeshComponent* BaseMeshTest = TankOwner->SmoothBoxTest;
	if (!BaseMeshTest) return;

	FVector CurrentBaseRelativeLocation = BaseMeshTest->GetRelativeLocation();

	if (!CurrentBaseRelativeLocation.IsNearlyZero())
	{
		FVector NewLocation = FMath::VInterpTo(CurrentBaseRelativeLocation, FVector::ZeroVector, DeltaTime, SmoothingSpeed);
		BaseMeshTest->SetRelativeLocation(NewLocation);

	}
	else
	{
		BaseMeshTest->SetRelativeLocation(FVector::ZeroVector);
		bIsVisualCorrectionActive = false;
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