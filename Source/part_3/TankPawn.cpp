
#include "TankPawn.h"

ATankPawn::ATankPawn()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CapsuleComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ATankPawn::Move(float Amount)
{
	FVector ForvardVector = CapsuleComponent->GetForwardVector();
	
	CurrentMoveAmount = FMath::FInterpTo(CurrentMoveAmount, Amount, GetWorld()->GetDeltaSeconds(), AccelerationDuration);

	UE_LOG(LogTemp, Warning, TEXT(" CurAmount: %f, Amount: %f, AccelDur: %f"), CurrentMoveAmount, Amount, AccelerationDuration);
	CapsuleComponent->AddForce(ForvardVector * CurrentMoveAmount * AccelerationForce * CapsuleComponent->GetMass());

	bMoveInputActive = true;
}

void ATankPawn::Turn(float Amount)
{
	CurrentTurnAmount = FMath::FInterpTo(CurrentTurnAmount, Amount, GetWorld()->GetDeltaSeconds(), RotationAccelerationDuration);

	FRotator Rotation = FRotator(0.f, CurrentTurnAmount, 0.f);
	UE_LOG(LogTemp, Warning, TEXT("Rotation: %f, CurAmount: %f"), Rotation.Yaw, CurrentTurnAmount);
	AddActorLocalRotation(Rotation);

	bTurnInputActive = true;
}

void ATankPawn::Look(const FInputActionValue& Amount)
{
	const FVector2D LookAxisValue = Amount.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
		UE_LOG(LogTemp, Warning, TEXT("x: %f, y: %f"), LookAxisValue.X, LookAxisValue.Y);
	}
}

void ATankPawn::Fire()
{

}

void ATankPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bMoveInputActive && CurrentMoveAmount != 0.f)
	{
		CurrentMoveAmount = FMath::FInterpTo(CurrentMoveAmount, 0.0f, DeltaTime, AccelerationDuration);
		UE_LOG(LogTemp, Warning, TEXT(" CurrentMoveAmount: %f, Interpolating to zero"), CurrentMoveAmount);
	}
	bMoveInputActive = false;


	if (!bTurnInputActive && CurrentTurnAmount != 0.f)
	{
		CurrentTurnAmount = FMath::FInterpTo(CurrentTurnAmount, 0.f, DeltaTime, RotationAccelerationDuration);
		UE_LOG(LogTemp, Warning, TEXT(" CurrentTurnAmount: %f, Interpolating to zero"), CurrentTurnAmount);
	}

	bTurnInputActive = false;
}
