
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
	SetIsReplicatedByDefault(true);
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

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, MaxHealth);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);

}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!ComponentOwner) return;

	if (ComponentOwner->HasAuthority())
	{
		if (Damage <= 0.0f)
		{
			return;
		}

		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
		//left here for standalone mode
		OnHealthChanged.Broadcast();

		if (CurrentHealth <= 0.f)
		{
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
		}
	}
}

void UHealthComponent::TakeDamage(float Damage, ETeam IncomingTeam)
{
	if (!ComponentOwner) return;

	if (ComponentOwner->HasAuthority())
	{
		if (Damage <= 0.0f)
		{
			return;
		}

		if (ComponentOwner->IsA<ATankPawn>())
		{
			if (IncomingTeam != ComponentOwnerTeam)
			{
				CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
				//left here for standalone mode
				OnHealthChanged.Broadcast();
			}
		}
		else
		{
			CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
			//left here for standalone mode
			OnHealthChanged.Broadcast();
		}


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
}

void UHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast();
}

void UHealthComponent::SetComponentOwnerTeam(ETeam NewOwnerTeam)
{
	ComponentOwnerTeam = NewOwnerTeam;
}

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}