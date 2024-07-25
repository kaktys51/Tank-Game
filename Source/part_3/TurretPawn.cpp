
#include "TurretPawn.h"

ATurretPawn::ATurretPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(CapsuleComponent);

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(CapsuleComponent);

	ProjectileSpawnPointFox = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPointFox->SetupAttachment(TurretMesh);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	MaterialColor = FLinearColor(1.0f, 1.0f, 1.0f);
}

void ATurretPawn::BeginPlay()
{
	Super::BeginPlay();
	
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

void ATurretPawn::RotateTurret(const FVector& LookAtTarget)
{
	if (TurretMesh)
	{
		FVector ToTarget = LookAtTarget - TurretMesh->GetComponentLocation();
		FRotator LookAtRotation = FRotator(0.f, ToTarget.Rotation().Yaw - 90.f, 0.f); // -90.f - adjust for turret position 
		FRotator NewLookRotation = FMath::RInterpTo(TurretMesh->GetComponentRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), TurretRotationAcceleration);
		TurretMesh->SetWorldRotation(NewLookRotation);
	}
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