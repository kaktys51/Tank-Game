#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerTeams_Enum.h"
#include "HealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnHealthChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PART_3_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

protected:

	virtual void BeginPlay() override;

	AActor* ComponentOwner;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	//This is copy of PawnTeam value for proper damage calculation
	UPROPERTY()
	ETeam ComponentOwnerTeam;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

public:	
	UFUNCTION()
	void TakeDamage(float Damage, ETeam IncomingTeam = ETeam::Blue);

	//Used for correct update call in UI health bar
	UFUNCTION()
	void OnRep_CurrentHealth();

	//Settup copy of PawnTeam to HealthComponent for proper damage calculation
	UFUNCTION()
	void SetComponentOwnerTeam(ETeam NewOwnerTeam);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	//Called when CurrentHealth changes its value
	FOnHealthChanged OnHealthChanged;

};
