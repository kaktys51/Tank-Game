
#pragma once

#include "CoreMinimal.h"
#include "TurretPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "TankPawn.generated.h"

UCLASS()
class PART_3_API ATankPawn : public ATurretPawn
{
	GENERATED_BODY()

public:

	ATankPawn();

protected:

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UCameraComponent> Camera;

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Move(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Turn(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Fire();

};