
#include "TankPawn.h"

ATankPawn::ATankPawn() : Super()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CapsuleComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ATankPawn::Move(float Amount)
{
	if (CapsuleComponent)
	{
		FVector ForvardVector = CapsuleComponent->GetForwardVector();

		CurrentMoveAmount = FMath::FInterpTo(CurrentMoveAmount, Amount, GetWorld()->GetDeltaSeconds(), AccelerationDuration);

		//UE_LOG(LogTemp, Warning, TEXT(" CurAmount: %f, Amount: %f, AccelDur: %f"), CurrentMoveAmount, Amount, AccelerationDuration);
		CapsuleComponent->AddForce(ForvardVector * CurrentMoveAmount * AccelerationForce * CapsuleComponent->GetMass());

		bMoveInputActive = true;
	}
}

void ATankPawn::Turn(float Amount)
{
	CurrentTurnAmount = FMath::FInterpTo(CurrentTurnAmount, Amount, GetWorld()->GetDeltaSeconds(), RotationAccelerationDuration);

	FRotator Rotation = FRotator(0.f, CurrentTurnAmount, 0.f);
	//UE_LOG(LogTemp, Warning, TEXT("Rotation: %f, CurAmount: %f"), Rotation.Yaw, CurrentTurnAmount);
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
		//UE_LOG(LogTemp, Warning, TEXT("x: %f, y: %f"), LookAxisValue.X, LookAxisValue.Y);
	}
}

void ATankPawn::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("Tank is shooting !"));
	if (ProjectileClass)
	{
		if (ProjectileSpawnPointFox)
		{
			FVector SpawnLocation = ProjectileSpawnPointFox->GetComponentLocation();
			FRotator SpawnRotation = ProjectileSpawnPointFox->GetComponentRotation();

			AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		}
	}
}

void ATankPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector ProjetileSpawnLocation = ProjectileSpawnPointFox->GetComponentLocation();
	DrawDebugSphere(GetWorld(), ProjetileSpawnLocation, 70.f, 12, FColor::Red);

	if (!bMoveInputActive && FMath::Abs(CurrentMoveAmount) > 0.1f)
	{
		CurrentMoveAmount = FMath::FInterpTo(CurrentMoveAmount, 0.0f, DeltaTime, AccelerationDuration);
	}
	bMoveInputActive = false;

	if (!bTurnInputActive && FMath::Abs(CurrentTurnAmount) != 0.1f)
	{
		CurrentTurnAmount = 0.f;
	}
	bTurnInputActive = false;
}
