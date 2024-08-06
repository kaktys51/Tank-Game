
#include "TowerPawn.h"
#include "TankPawn.h"

ATowerPawn::ATowerPawn()
{
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
	DetectionSphere->SetupAttachment(CapsuleComponent);

	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ATowerPawn::OnOverlapBegin);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ATowerPawn::OnOverlapEnd);
}

void ATowerPawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ATankPawn::StaticClass()))
	{
		TargetRef = OtherActor;

		FVector TargetLocation = TargetRef->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT(" TargetLocation: %f, %f, %f"), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
		RotateTurret(TargetLocation);
		GetWorld()->GetTimerManager().SetTimer(ShootingTimer, this, &ATowerPawn::Fire, ShootInterval, true);
	}
}

void ATowerPawn::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == TargetRef)
	{
		TargetRef = nullptr;
		GetWorld()->GetTimerManager().ClearTimer(ShootingTimer);
	}
}

void ATowerPawn::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("Tower is shooting at player!"));

	if (ProjectileClass && ProjectileSpawnPointFox)
	{
		FVector SpawnLocation = ProjectileSpawnPointFox->GetComponentLocation();
		FRotator SpawnRotation = ProjectileSpawnPointFox->GetComponentRotation();

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);

		FireVFX();
	}
}

void ATowerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetRef)
	{
		FVector TargetLocation = TargetRef->GetActorLocation();
		RotateTurret(TargetLocation);
	}
}
