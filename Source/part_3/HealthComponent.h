#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

public:	
	UFUNCTION()
	void TakeDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	//Called when CurrentHealth changes its value
	FOnHealthChanged OnHealthChanged;

};
