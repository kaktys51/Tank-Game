
#include "TankMovementComponent.h"
#include "TankPawn.h"

UTankMovementComponent::UTankMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

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
}

void UTankMovementComponent::Turn(float Amount)
{
	CurrentTurnAmount = FMath::FInterpTo(CurrentTurnAmount, Amount, GetWorld()->GetDeltaSeconds(), RotationAccelerationDuration);

	FRotator Rotation = FRotator(0.f, CurrentTurnAmount, 0.f);
	TankOwner->AddActorLocalRotation(Rotation);

	bTurnInputActive = true;
}
