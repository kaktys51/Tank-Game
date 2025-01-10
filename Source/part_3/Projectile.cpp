
#include "Projectile.h"
#include "HealthComponent.h"
#include "TurretPawn.h"

AProjectile::AProjectile()
{
	//bReplicates = true;
	//SetReplicateMovement(true);

	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	ProjectileMesh->SetCollisionProfileName(TEXT("BlockAll"));
	ProjectileMesh->OnComponentHit.AddDynamic(this, &AProjectile::OnComponentHit);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = ProjectileMesh;

	DamageValue = 20.f;
}

void AProjectile::SetOwnerTeam(ETeam TeamValue)
{
	OwnerTeam = TeamValue;
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile hit: %s"), *OtherActor->GetName());
		if (HasAuthority())
		{
			UHealthComponent* HealthComponent = OtherActor->FindComponentByClass<UHealthComponent>();
			{
				if (HealthComponent)
				{
					HealthComponent->TakeDamage(DamageValue, OwnerTeam);
				}
			}
		}

		FVector HitLocation = Hit.ImpactPoint;
		FRotator HitRotation = UKismetMathLibrary::MakeRotFromZ(-Hit.Normal);
		if (HitEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, HitLocation, HitRotation, true);
		}

		//Plays hit sound base on OtherActor class
		if (OtherActor->IsA(ATurretPawn::StaticClass()))
		{
			UGameplayStatics::PlaySoundAtLocation(this, TurretHitSound, HitLocation);
		}
		else
		{
			UGameplayStatics::PlaySoundAtLocation(this, GeneralHitSound, HitLocation);
		}

		Destroy();
	}
}
