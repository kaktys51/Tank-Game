
#include "TurretPawn.h"
#include "TankPawn.h"


ATurretPawn::ATurretPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(RootComponent);

	ProjectileSpawnPointFox = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPointFox->SetupAttachment(TurretMesh);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	MaterialColor = FLinearColor(1.0f, 1.0f, 1.0f);
}

void ATurretPawn::BeginPlay()
{
	Super::BeginPlay();

}

void ATurretPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATurretPawn, CapsuleComponent);
	DOREPLIFETIME(ATurretPawn, BaseMesh);
	DOREPLIFETIME(ATurretPawn, TurretMesh);
	DOREPLIFETIME(ATurretPawn, ProjectileSpawnPointFox);
	DOREPLIFETIME(ATurretPawn, HealthComponent);
	DOREPLIFETIME(ATurretPawn, TurretRotationAcceleration);
	DOREPLIFETIME(ATurretPawn, DestroyedClass);
}

TArray<FString> ATurretPawn::GetMaterialParametrs() const
{
	TArray<FString> LocalMaterialParameters;

	LocalMaterialParameters.Add("TeamColor");

	return LocalMaterialParameters;
}

TArray<FName> ATurretPawn::GetSlotNames() const
{
	TArray<FName> BaseMaterialSlotNames;

	if (BaseMesh)
	{
		BaseMaterialSlotNames = BaseMesh->GetMaterialSlotNames();
	}

	return BaseMaterialSlotNames;
}

void ATurretPawn::TurretRotationToCursor()
{

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		FHitResult HitResult;
		if (PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			FVector HitLocation = HitResult.ImpactPoint;
			RotateTurret(HitLocation);
			DrawDebugSphere(GetWorld(), HitLocation, 70.f, 12, FColor::Red);
		}
	}
}

void ATurretPawn::ServerTurretRotationToCursor_Implementation()
{
	TurretRotationToCursor();
}


void ATurretPawn::RotateTurret(const FVector& LookAtTarget)
{
	if (HasAuthority())
	{
		if (TurretMesh)
		{
			FVector ToTarget = LookAtTarget - TurretMesh->GetComponentLocation();

			FRotator CurrentRotation = TurretMesh->GetRelativeRotation();
			FRotator TankRotation = GetActorRotation();

			FRotator LookAtRotation = FRotator(0.f, ToTarget.Rotation().Yaw - 90.f, 0.f); // -90.f - adjust for turret position 
			LookAtRotation.Yaw -= TankRotation.Yaw;

			FRotator NewLookRotation = FMath::RInterpConstantTo(CurrentRotation, LookAtRotation, GetWorld()->GetDeltaSeconds(), TurretRotationAcceleration);

			TurretMesh->SetRelativeRotation(NewLookRotation);
		}
	}
	else
	{
		ServerRotateTurret(LookAtTarget);
	}
}

void ATurretPawn::ServerRotateTurret_Implementation(const FVector& LookAtTarget)
{
	RotateTurret(LookAtTarget);
}


void ATurretPawn::HandleDeath()
{
	if (HasAuthority())
	{
		if (BaseMesh && TurretMesh && CapsuleComponent)
		{
			BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			TurretMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			FVector CurrentLocation = GetActorLocation();
			FRotator CurrentRotation = FRotator::ZeroRotator;
			if (DestroyedClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				ADestroyedPawn* DestroyedPawn = GetWorld()->SpawnActor<ADestroyedPawn>(DestroyedClass, CurrentLocation, CurrentRotation, SpawnParams);
				if (BaseMesh && TurretMesh && DestroyedPawn)
				{
					DestroyedPawn->SetBasePosition(BaseMesh->GetComponentTransform());
					DestroyedPawn->SetTurretPosition(TurretMesh->GetComponentTransform());
				}
			}
		}

		Destroy();
	}
}

void ATurretPawn::ServerHandleDeath_Implementation()
{
	HandleDeath();
}

void ATurretPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATurretPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UStaticMeshComponent* StaticBaseMash = BaseMesh;
	int32 SlotIndex = StaticBaseMash->GetMaterialIndex(MaterialSlotName);

	if (SlotIndex != INDEX_NONE)
	{
		UMaterialInterface* MaterialInterface = StaticBaseMash->GetMaterial(SlotIndex);
		if (MaterialInterface)
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialInterface, StaticBaseMash);

			FName MaterialParametrName = MaterialParametrs;
			FLinearColor NewParametrColor = MaterialColor;
			DynamicMaterial->SetVectorParameterValue(MaterialParametrName, NewParametrColor);

			if (BaseMesh && TurretMesh)
			{
				BaseMesh->SetMaterial(SlotIndex, DynamicMaterial);
				TurretMesh->SetMaterial(SlotIndex, DynamicMaterial);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Material not found"));
	}
}