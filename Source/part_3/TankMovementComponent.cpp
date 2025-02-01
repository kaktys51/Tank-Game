
#include "TankMovementComponent.h"
#include "TankPawn.h"

UTankMovementComponent::UTankMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

FSaveMove::FSaveMove()
{
}

void UTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	PawnOwner = GetPawnOwner();
	TankOwner = Cast<ATankPawn>(PawnOwner);
}

void UTankMovementComponent::Move(float Amount)
{
	FVector ForvardVector = TankOwner->CapsuleComponent->GetForwardVector();

	CurrentMoveAmount = FMath::FInterpTo(CurrentMoveAmount, Amount, GetWorld()->GetDeltaSeconds(), AccelerationDuration);
	TankOwner->CapsuleComponent->AddForce(ForvardVector * CurrentMoveAmount * AccelerationForce * TankOwner->CapsuleComponent->GetMass());
	
	bMoveInputActive = true;

	FVector ActorLocation = TankOwner->GetActorLocation();
	FVector TankVelocity = TankOwner->CapsuleComponent->GetPhysicsLinearVelocity();
	FRotator Rotation = TankOwner->GetActorRotation();

	FSaveMove MoveData(GetWorld()->GetTimeSeconds(),CurrentMoveAmount, TankOwner->GetActorLocation(), TankVelocity, Rotation);

	PendingMoves.Add(MoveData);

	if (!TankOwner->HasAuthority())
	{
		Server_SendMove(MoveData);
	}
}

void UTankMovementComponent::Server_SendMove_Implementation(const FSaveMove& MoveData)
{
	UE_LOG(LogTemp, Warning, TEXT("TimeStamp: %f"), MoveData.TimeStamp);
	FVector ForvardVector = TankOwner->CapsuleComponent->GetForwardVector();

	CurrentMoveAmount = MoveData.Saved_CurrentMoveAmount;
	TankOwner->CapsuleComponent->AddForce(ForvardVector * CurrentMoveAmount * AccelerationForce * TankOwner->CapsuleComponent->GetMass());

	FVector AuthVelocity = TankOwner->CapsuleComponent->GetPhysicsLinearVelocity();
	FVector AuthLocation = TankOwner->GetActorLocation();
	FRotator AuthRotation = TankOwner->GetActorRotation();
	float DistanceError = FVector::DistSquared(AuthLocation, MoveData.Saved_MovedLocation);

	UE_LOG(LogTemp, Warning, TEXT("Server location: %f, %f, %f"), AuthLocation.X, AuthLocation.Y, AuthLocation.Z);
	UE_LOG(LogTemp, Warning, TEXT("Client location: %f, %f, %f"), MoveData.Saved_MovedLocation.X, MoveData.Saved_MovedLocation.Y, MoveData.Saved_MovedLocation.Z);

	float CorrectionValue = FMath::Square(DistanceCorrection);

	UE_LOG(LogTemp, Warning, TEXT("DistanceError: %f"), DistanceError);

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

	PendingMoves.Empty();
}

void UTankMovementComponent::Turn(float Amount)
{
	CurrentTurnAmount = FMath::FInterpTo(CurrentTurnAmount, Amount, GetWorld()->GetDeltaSeconds(), RotationAccelerationDuration);

	FRotator Rotation = FRotator(0.f, CurrentTurnAmount, 0.f);
	TankOwner->AddActorLocalRotation(Rotation);

	bTurnInputActive = true;
}
