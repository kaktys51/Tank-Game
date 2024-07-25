
#pragma once

#include "CoreMinimal.h"
#include "TurretPawn.h"
#include "Projectile.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "TankPawn.generated.h"

UCLASS()
class PART_3_API ATankPawn : public ATurretPawn
{
	GENERATED_BODY()

public:

	ATankPawn();

protected:
	float CurrentMoveAmount = 0.f;
	float CurrentTurnAmount = 0.f;

	bool bMoveInputActive = false;
	bool bTurnInputActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bTurretToCursorState;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AccelerationForce = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AccelerationDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationAccelerationDuration = 0.2f;

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Move(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Turn(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Look(const FInputActionValue& Amount);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Fire();

	//Disables or enables turret rotation to cursor
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetTurretRotationToCursorState(bool bInputState);

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AProjectile> ProjectileClass;

	virtual void Tick(float DeltaTime) override;
};