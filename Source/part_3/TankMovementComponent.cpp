
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

	if (TankOwner->HasAuthority())
	{
		SimProxyTransform = TankOwner->GetActorTransform();
	}

	SpringArmOrigin = TankOwner->SpringArm->GetRelativeTransform();

	// test visual smoothing setup
	BodyVisualLocation = TankOwner->GetActorLocation();
	TankOwner->SmoothBoxTest->SetWorldLocation(BodyVisualLocation);
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
		NewMove.TimeStamp = GetWorld()->GetTimeSeconds();
		NewMove.DeltaTime = DeltaTime;
		NewMove.InputVector = PendingInput;
		NewMove.SavedLocation = TankOwner->GetActorLocation();

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

	FVector OwnerLocation = TankOwner->GetActorLocation();

	//UE_LOG(LogTemp, Warning, TEXT("BoxTestLocation: %f, %f, %f "), BoxTestLocation.X, BoxTestLocation.Y, BoxTestLocation.Z);

	FVector NewLocation = FMath::VInterpTo(BodyVisualLocation, OwnerLocation, DeltaTime, VisualInterpolationSpeed);

	TankOwner->SmoothBoxTest->SetWorldLocation(NewLocation);

	BodyVisualLocation = NewLocation;
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

	FVector ServerLocation = PawnOwner->GetActorLocation();
	FVector ClientLocation = MoveData.SavedLocation;

	// Cheking distance betven server calculation and client data
	float DistanceError = FVector::DistSquared(ServerLocation, ClientLocation);
	float CorrectionValue = FMath::Square(DistanceCorrection);

	UE_LOG(LogTemp, Warning, TEXT("DistanceError: %f"), DistanceError);
	if (DistanceError >= CorrectionValue)
	{
		Multicast_CorrectionData(UpdatedComponent->GetComponentTransform(), MoveData.TimeStamp);
	}
}

void UTankMovementComponent::Multicast_CorrectionData_Implementation(const FTransform& ServerTransform, float TimeStamp)
{
	if (!TankOwner->HasAuthority())
	{
		//// Saving old location for smoothe intep
		//UStaticMeshComponent* BaseMeshTest = TankOwner->SmoothBoxTest;

		//PreviousTransformBox = BaseMeshTest->GetComponentTransform();
		

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

		//// Saving latest corrected location form smoother transform
		//TargetTransformBox = BaseMeshTest->GetComponentTransform();

		//// Calculating offset for further smoothing
		//OffsetBoxLocation = PreviousTransformBox.GetLocation() - TargetTransformBox.GetLocation();
		//
		//BaseMeshTest->SetRelativeLocation(OffsetBoxLocation);
		//bIsVisualCorrectionActive = true;
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