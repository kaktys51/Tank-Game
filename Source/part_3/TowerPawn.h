
#pragma once

#include "CoreMinimal.h"
#include "TurretPawn.h"
#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "TowerPawn.generated.h"


UCLASS()
class PART_3_API ATowerPawn : public ATurretPawn
{
	GENERATED_BODY()
	
public:

	ATowerPawn();

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	AActor* TargetRef;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USphereComponent> DetectionSphere;

	UPROPERTY(BlueprintReadOnly, Category = "Tank Actions")
	bool bGunLoaded = true;

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void ReloadGun();

	UFUNCTION(BlueprintCallable, Category = "Tower Actions")
	void Fire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireVSFX();

	//For implementing vfx inside blueprint
	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void FireVSFX();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AProjectile> ProjectileClass;

	FTimerHandle ReloadTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
	float ReloadTime = 2.f;

	virtual void Tick(float DeltaTime) override;
};
