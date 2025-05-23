
#include "TankPawn.h"
#include "CustomPlayerController.h"


ATankPawn::ATankPawn() : Super()
{

	bReplicates = true;
	SetReplicateMovement(false);

	TankVisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TankVisualRoot"));

	BaseMesh->SetupAttachment(TankVisualRoot);
	TurretMesh->SetupAttachment(TankVisualRoot);

	GroundSampleFL = CreateDefaultSubobject<USceneComponent>(TEXT("GroundSampleFL"));
	GroundSampleFL->SetupAttachment(TankVisualRoot);

	GroundSampleFR = CreateDefaultSubobject<USceneComponent>(TEXT("GroundSampleFR"));
	GroundSampleFR->SetupAttachment(TankVisualRoot);

	GroundSampleBL = CreateDefaultSubobject<USceneComponent>(TEXT("GroundSampleBL"));
	GroundSampleBL->SetupAttachment(TankVisualRoot);

	GroundSampleBR = CreateDefaultSubobject<USceneComponent>(TEXT("GroundSampleBR"));
	GroundSampleBR->SetupAttachment(TankVisualRoot);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	//SpringArm->SetupAttachment(CapsuleComponent);
	SpringArm->SetupAttachment(TankVisualRoot);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	MovementComponent = CreateDefaultSubobject<UTankMovementComponent>(TEXT("TankMovementComponent"));
	MovementComponent->UpdatedComponent = CapsuleComponent;

	bTurretToCursorState = true;

}

void ATankPawn::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddUObject(this, &ATankPawn::HealthUpdated);
	}

	//used for applying correct team color of simProxy on client side when joined the game
	if (!IsLocallyControlled())
	{
		if (DynamicTeamColor)
		{
			DynamicTeamColor->SetVectorParameterValue(MaterialParametrs, MaterialColor);
		}
	}

	// Setting up VisualRoot to ActorRoot when game starting 
	TankVisualRoot->SetWorldTransform(GetTransform());
}

void ATankPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATankPawn, MovementComponent);
	DOREPLIFETIME(ATankPawn, bTurretToCursorState);
	DOREPLIFETIME(ATankPawn, bGunLoaded);
	DOREPLIFETIME(ATankPawn, PawnTeam);
	DOREPLIFETIME_CONDITION(ATankPawn, SimTankVelocity, COND_SimulatedOnly);
}

void ATankPawn::MoveComp(float Amount)
{
	/*if (MovementComponent)
	{
		MovementComponent->TankOwner;
		MovementComponent->Move(Amount);
	}*/

	if (MovementComponent)
	{
		FVector ForvardVector = CapsuleComponent->GetForwardVector();
		MovementComponent->AddMoveInputVector(ForvardVector * Amount);
	}
}

void ATankPawn::TurnComp(float Amount)
{
	if (MovementComponent)
	{
		MovementComponent->AddTurnValue(Amount);
	}
}


void ATankPawn::Move(float Amount)
{
	if(HasAuthority())
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
	else
	{
		ServerMove(Amount);
	}
}

void ATankPawn::ServerMove_Implementation(float Amount)
{
	Move(Amount);
}


void ATankPawn::Turn(float Amount)
{
	if (HasAuthority())
	{
		CurrentTurnAmount = FMath::FInterpTo(CurrentTurnAmount, Amount, GetWorld()->GetDeltaSeconds(), RotationAccelerationDuration);

		FRotator Rotation = FRotator(0.f, CurrentTurnAmount, 0.f);
		//UE_LOG(LogTemp, Warning, TEXT("Rotation: %f, CurAmount: %f"), Rotation.Yaw, CurrentTurnAmount);
		AddActorLocalRotation(Rotation);

		bTurnInputActive = true;
	}
	else
	{
		ServerTurn(Amount);
	}
}

void ATankPawn::ServerTurn_Implementation(float Amount)
{
	Turn(Amount);
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
	if(HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tank is shooting !"));
		if (ProjectileClass && ProjectileSpawnPointFox)
		{
			if (!bGunLoaded) return;

			bGunLoaded = false;
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &ATankPawn::ReloadGun, ReloadTime, false);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = this->GetInstigator();

			FVector SpawnLocation = ProjectileSpawnPointFox->GetComponentLocation();
			FRotator SpawnRotation = ProjectileSpawnPointFox->GetComponentRotation();

			AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
			Projectile->SetOwnerTeam(PawnTeam);

			MulticastFireVSFX();
			//FireVSFX();
		}
	}
	else
	{
		ServerFire();
	}
}

void ATankPawn::ServerFire_Implementation()
{
	Fire();
}

void ATankPawn::MulticastFireVSFX_Implementation()
{
	if (!HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this->GetInstigator();

		FVector SpawnLocation = ProjectileSpawnPointFox->GetComponentLocation();
		FRotator SpawnRotation = ProjectileSpawnPointFox->GetComponentRotation();

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	FireVSFX();
}

void ATankPawn::ReloadGun()
{
	bGunLoaded = true;
}

//Disables or enables turret rotation to cursor
void ATankPawn::SetTurretRotationToCursorState(bool bInputState)
{
	bTurretToCursorState = bInputState;
}


void ATankPawn::SetTeamSettings(FLinearColor NewTeamColor, ETeam NewTeam)
{
	//for correct exec need to set MaterialSlotName and MaterialParametrs in editor properly

	MaterialColor = NewTeamColor;
	PawnTeam = NewTeam;
	HealthComponent->SetComponentOwnerTeam(NewTeam);

	if (DynamicTeamColor)
	{
		DynamicTeamColor->SetVectorParameterValue(MaterialParametrs, MaterialColor);
	}

	MulticastColorSettings(NewTeamColor);
}

void ATankPawn::MulticastColorSettings_Implementation(FLinearColor NewTeamColor)
{
	if (!HasAuthority())
	{
		MaterialColor = NewTeamColor;
		if (DynamicTeamColor)
		{
			DynamicTeamColor->SetVectorParameterValue(MaterialParametrs, MaterialColor);
		}
	}
}

void ATankPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController)
	{
		ACustomPlayerController* TankController = Cast<ACustomPlayerController>(NewController);
		if (TankController)
		{
			TankController->SetPawnTeam(this);
		}
	}
}

void ATankPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTurretToCursorState)
	{
		TurretRotationToCursor();
	}

	FVector ProjetileSpawnLocation = ProjectileSpawnPointFox->GetComponentLocation();
	DrawDebugSphere(GetWorld(), ProjetileSpawnLocation, 70.f, 12, FColor::Red);

	// off until tankMovementComponent !!!!!!
	// 
	//if (!bMoveInputActive && FMath::Abs(CurrentMoveAmount) > 0.1f)
	//{
	//	CurrentMoveAmount = FMath::FInterpTo(CurrentMoveAmount, 0.0f, DeltaTime, AccelerationDuration);
	//}
	//bMoveInputActive = false;

	//if (!bTurnInputActive && FMath::Abs(CurrentTurnAmount) != 0.1f)
	//{
	//	CurrentTurnAmount = 0.f;
	//}
	//bTurnInputActive = false;
}
