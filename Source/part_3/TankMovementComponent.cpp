
#include "TankMovementComponent.h"
#include "TankPawn.h"

UTankMovementComponent::UTankMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

FSaveMove::FSaveMove()
{
}

FSaveNetData::FSaveNetData()
{
}

FTankSafeMove::FTankSafeMove()
{
}

void UTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	PawnOwner = GetPawnOwner();
	TankOwner = Cast<ATankPawn>(PawnOwner);
}


void UTankMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent)
	{
		return;
	}

	if (!PendingInput.IsNearlyZero())
	{
		if (TankOwner->HasAuthority()) { return; }

		UE_LOG(LogTemp, Warning, TEXT("Server MoveTick"));
		//FVector BeforeLoc = TankOwner->GetActorLocation();
		//UE_LOG(LogTemp, Warning, TEXT("Before: %f, %f, %f"), BeforeLoc.X, BeforeLoc.Y, BeforeLoc.Z);

		// Calculate movement distance 
		FVector DesiredMovement = PendingInput.GetClampedToMaxSize(1.0f) * MoveSpeed * DeltaTime;

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
		FTankSafeMove NewMove(DeltaTime, GetWorld()->GetTimeSeconds(), PendingInput, TankOwner->GetActorLocation());
		SavedMoves.Add(NewMove);

		//if (PawnOwner->IsLocallyControlled())
		//{
		//	Server_Move(NewMove);
		//}

		ENetRole LocalRole = GetOwner()->GetLocalRole();
		if (LocalRole == ROLE_AutonomousProxy)
		{
			Server_Move(NewMove);
			UE_LOG(LogTemp, Warning, TEXT("Client send"));
		}
	}

	PendingInput = FVector::ZeroVector;
}

void UTankMovementComponent::AddInputVector(const FVector& WorldVector)
{
	PendingInput += WorldVector;
}

void UTankMovementComponent::Server_Move_Implementation(const FTankSafeMove& MoveData)
{
	UE_LOG(LogTemp, Warning, TEXT("Server exec"));
	FVector DesiredMovement = MoveData.InputVector.GetClampedToMaxSize(1.0f) * MoveSpeed * MoveData.DeltaTime;
	FHitResult Hit;
	SafeMoveUpdatedComponent(DesiredMovement, UpdatedComponent->GetComponentRotation(), true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		SlideAlongSurface(DesiredMovement, 1.0f - Hit.Time, Hit.Normal, Hit);
	}

	FVector ServerLocation = PawnOwner->GetActorLocation();
	FVector ClientLocation = MoveData.SavedLocation;

	// Cheking distance betven server calculation and client data
	float DistanceError = FVector::DistSquared(ServerLocation, ClientLocation);
	float CorrectionValue = FMath::Square(DistanceCorrection);

}

void UTankMovementComponent::Move(float Amount)
{
	FVector ForvardVector = TankOwner->CapsuleComponent->GetForwardVector();

	CurrentMoveAmount = FMath::FInterpTo(CurrentMoveAmount, Amount, GetWorld()->GetDeltaSeconds(), AccelerationDuration);
	TankOwner->CapsuleComponent->AddForce(ForvardVector * CurrentMoveAmount * AccelerationForce * TankOwner->CapsuleComponent->GetMass());
	

	//only for testing
	//CurrentMoveAmount = 0.f;

	bMoveInputActive = true;

	FVector ActorLocation = TankOwner->GetActorLocation();
	FVector TankVelocity = TankOwner->CapsuleComponent->GetPhysicsLinearVelocity();
	FRotator Rotation = TankOwner->GetActorRotation();

	FSaveMove MoveData(GetWorld()->GetTimeSeconds(),CurrentMoveAmount, TankOwner->GetActorLocation(), TankVelocity, Rotation);
	FSaveNetData NetData(GetWorld()->GetTimeSeconds(), CurrentMoveAmount, TankOwner->GetActorLocation(), Rotation);

	PendingMoves.Add(MoveData);

	if (!TankOwner->HasAuthority())
	{
		Server_SendMove(MoveData, NetData);
	}
}

void UTankMovementComponent::Server_SendMove_Implementation(const FSaveMove& MoveData, const FSaveNetData& NetData)
{
	UE_LOG(LogTemp, Warning, TEXT("TimeStamp: %f"), MoveData.TimeStamp);
	FVector ForvardVector = TankOwner->CapsuleComponent->GetForwardVector();

	
	CurrentMoveAmount = MoveData.Saved_CurrentMoveAmount;
	//CurrentMoveAmount = NetData.Net_CurrentMoveAmount / 255.f;
	TankOwner->CapsuleComponent->AddForce(ForvardVector * CurrentMoveAmount * AccelerationForce * TankOwner->CapsuleComponent->GetMass());

	//only for testing
	//CurrentMoveAmount = 0.f;

	FVector AuthVelocity = TankOwner->CapsuleComponent->GetPhysicsLinearVelocity();
	FVector AuthLocation = TankOwner->GetActorLocation();
	FRotator AuthRotation = TankOwner->GetActorRotation();

	float DistanceError = FVector::DistSquared(AuthLocation, MoveData.Saved_MovedLocation);
	//float DistanceError = FVector::DistSquared(AuthLocation, NetData.Net_MovedLocation);

	//UE_LOG(LogTemp, Warning, TEXT("Server location: %f, %f, %f"), AuthLocation.X, AuthLocation.Y, AuthLocation.Z);
	//UE_LOG(LogTemp, Warning, TEXT("Client location: %f, %f, %f"), MoveData.Saved_MovedLocation.X, MoveData.Saved_MovedLocation.Y, MoveData.Saved_MovedLocation.Z);

	float CorrectionValue = FMath::Square(DistanceCorrection);

	UE_LOG(LogTemp, Warning, TEXT("DistanceError: %f"), DistanceError);

	//Send correction if needed 
	if (DistanceError >= CorrectionValue)
	{
		if(TankOwner->HasAuthority())
		{
			Multicast_SendCorrectionData(MoveData.TimeStamp, AuthLocation, AuthVelocity, AuthRotation);
		}
	}

}

void UTankMovementComponent::Multicast_SendCorrectionData_Implementation(float TimeStamp, FVector CorrectedLoacation, FVector CorrectedVelocity, FRotator CorrectedRotation)
{
	//if (TankOwner->IsLocallyControlled()) return;

	ENetRole LocalRole = GetOwner()->GetLocalRole();

	if (LocalRole == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("Executing on the SERVER (Authority)"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Executing on the CLIENT (Simulated)"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Client Corrected!!!"));
	//if (!TankOwner->HasAuthority())
	if(!TankOwner->HasAuthority())
	{
		TankOwner->SetActorLocation(CorrectedLoacation);
		TankOwner->SetActorRotation(CorrectedRotation);
		TankOwner->CapsuleComponent->SetPhysicsLinearVelocity(CorrectedVelocity);

	}

	//PendingMoves.Empty();

	//TArray<FSaveMove> MovesToReplay;
	//for (const FSaveMove& Move : PendingMoves)
	//{
	//	if (Move.TimeStamp > TimeStamp)
	//	{
	//		MovesToReplay.Add(Move);
	//	}
	//}
	//PendingMoves = MovesToReplay;

	//// Пересчитываем оставшиеся ходы (replay)
	//for (const FSaveMove& Move : PendingMoves)
	//{
	//	FVector ForwardVector = TankOwner->CapsuleComponent->GetForwardVector();
	//	TankOwner->CapsuleComponent->AddForce(ForwardVector * Move.Saved_CurrentMoveAmount * AccelerationForce * TankOwner->CapsuleComponent->GetMass());
	//}

}

void UTankMovementComponent::Turn(float Amount)
{
	CurrentTurnAmount = FMath::FInterpTo(CurrentTurnAmount, Amount, GetWorld()->GetDeltaSeconds(), RotationAccelerationDuration);

	FRotator Rotation = FRotator(0.f, CurrentTurnAmount, 0.f);
	TankOwner->AddActorLocalRotation(Rotation);

	bTurnInputActive = true;
}
