
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TankMovementComponent.generated.h"

class ATankPawn;

UCLASS()
class PART_3_API UTankMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	
public:

	UTankMovementComponent();

	virtual void BeginPlay() override;

	//Interpolated coefficient used in AddFocre for smoothe movement. Important for proper replication!
	float CurrentMoveAmount = 0.f;
	float CurrentTurnAmount = 0.f;

	bool bMoveInputActive = false;
	bool bTurnInputActive = false;

	APawn* PawnOwner;
	ATankPawn* TankOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AccelerationForce = 1000.f;

	//Changes the speed of interpolation of acceleration 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AccelerationDuration = 0.2f;

	//Changes the speed of interpolation of hull rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationAccelerationDuration = 0.2f;

	//Movement func

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Move(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Turn(float Amount);
};
