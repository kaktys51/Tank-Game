
#include "HealthComponent.h"
#include "TankPawn.h"
#include "TowerPawn.h"
#include "GameModeBaseFox.h"
#include "Kismet/GameplayStatics.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;
	ComponentOwner = nullptr;
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	ComponentOwner = GetOwner();

	if (ComponentOwner)
	{
		ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
		UE_LOG(LogTemp, Warning, TEXT("BeginPlay UHealthComponent"));
	}
}


void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		if (GetOwner())
		{
			GetOwner()->Destroy();
		}
	}
}

void UHealthComponent::TakeDamage(float Damage)
{
	if (Damage <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

	if (CurrentHealth > 0.0f) return;

	if (ComponentOwner->IsA<ATankPawn>())
	{
		ATankPawn* TankPawn = Cast<ATankPawn>(ComponentOwner);
		if (TankPawn)
		{
			AGameModeBaseFox* GameMode = Cast<AGameModeBaseFox>(UGameplayStatics::GetGameMode(this));
			if (GameMode)
			{
				GameMode->LoseGame();
				TankPawn->HandleDeath();
			}
		}
	}
	else if (ComponentOwner->IsA<ATowerPawn>())
	{
		ATowerPawn* TowerPawn = Cast<ATowerPawn>(ComponentOwner);
		if (TowerPawn)
		{
			AGameModeBaseFox* GameMode = Cast<AGameModeBaseFox>(UGameplayStatics::GetGameMode(this));
			if (GameMode)
			{
				GameMode->AddScore();
				TowerPawn->HandleDeath();
			}

		}

	}

}