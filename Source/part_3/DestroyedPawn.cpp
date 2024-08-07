
#include "DestroyedPawn.h"
#include "CustomPlayerController.h"
#include <Kismet/GameplayStatics.h>

ADestroyedPawn::ADestroyedPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(RootComponent);

	MainExplosionLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MainExplosionEmiterLocation"));
	MainExplosionLocation->SetupAttachment(RootComponent);

	OffsetMainExplosionLocation = 0.f;
	TurretOffset = 10.0f;
}

void ADestroyedPawn::SetMainExplosionLocation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController) return;

	ACustomPlayerController* CustomPlayerController = Cast<ACustomPlayerController>(PlayerController);
	if (!CustomPlayerController) return;

	FVector DestroyedPawnLocation = MainExplosionLocation->GetComponentLocation();
	if (!MainExplosionLocation) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	CustomPlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	if (DestroyedPawnLocation.IsNearlyZero(0.1) && CameraLocation.IsNearlyZero(0.1) && CameraRotation.IsNearlyZero(0.1)) return;

	FVector Direction = (DestroyedPawnLocation - CameraLocation).GetSafeNormal();

	FVector AdjustedLocation = DestroyedPawnLocation - Direction * OffsetMainExplosionLocation;

	MainExplosionLocation->SetWorldLocation(AdjustedLocation);
}

void ADestroyedPawn::SetTurretPosition(const FTransform& TurretTransform)
{
	if (TurretMesh)
	{
		TurretMesh->SetWorldTransform(TurretTransform);

		FVector CurrentLocation = TurretMesh->GetComponentLocation();
		CurrentLocation.Z += TurretOffset;
		TurretMesh->SetWorldLocation(CurrentLocation);
	}
}

void ADestroyedPawn::SetBasePosition(const FTransform& BaseTransform)
{
	if (BaseMesh)
	{
		BaseMesh->SetWorldTransform(BaseTransform);
	}
}

void ADestroyedPawn::BeginPlay()
{
	Super::BeginPlay();

	SetMainExplosionLocation();
}
