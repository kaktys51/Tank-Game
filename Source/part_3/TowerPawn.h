
#pragma once

#include "CoreMinimal.h"
#include "TurretPawn.h"
#include "HealthComponent.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	UHealthComponent* HealthComponent;

	UFUNCTION(BlueprintCallable, Category = "Tower Actions")
	void Fire();

	FTimerHandle ShootingTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
	float ShootInterval = 2.f;

	virtual void Tick(float DeltaTime) override;
};
