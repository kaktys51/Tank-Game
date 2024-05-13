

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
}

void ATankPawn::Turn(float Amount)
{
}

void ATankPawn::Fire()
{
}
