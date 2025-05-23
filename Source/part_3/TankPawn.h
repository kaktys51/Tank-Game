
#pragma once

#include "CoreMinimal.h"
#include "TurretPawn.h"
#include "Projectile.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "PlayerTeams_Enum.h"
#include "InputActionValue.h"
#include "TankMovementComponent.h"
#include "TankPawn.generated.h"

UCLASS()
class PART_3_API ATankPawn : public ATurretPawn
{
	GENERATED_BODY()

public:

	ATankPawn();

protected:

	virtual void BeginPlay() override;

	float CurrentMoveAmount = 0.f;
	float CurrentTurnAmount = 0.f;

	bool bMoveInputActive = false;
	bool bTurnInputActive = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement")
	bool bTurretToCursorState;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UTankMovementComponent> MovementComponent;


	//**************************************************************************
	// 

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USceneComponent> TankVisualRoot;

	// List of components used to calculate line trace for proper tank hull to surface alignment

	UPROPERTY(EditInstanceOnly, Category = "Tank|GroundAlignment")
	TObjectPtr<USceneComponent> GroundSampleFL;  // Front Left

	UPROPERTY(EditInstanceOnly, Category = "Tank|GroundAlignment")
	TObjectPtr<USceneComponent> GroundSampleFR;  // Front Right

	UPROPERTY(EditInstanceOnly, Category = "Tank|GroundAlignment")
	TObjectPtr<USceneComponent> GroundSampleBL;  // Back Left

	UPROPERTY(EditInstanceOnly, Category = "Tank|GroundAlignment")
	TObjectPtr<USceneComponent> GroundSampleBR;  // Back Right

	//**************************************************************************


	//Timer for handle reload
	FTimerHandle ReloadTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank Actions")
	float ReloadTime = 2.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tank Actions")
	bool bGunLoaded = true;

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void ReloadGun();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AccelerationForce = 1000.f;

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

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Look(const FInputActionValue& Amount);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Fire();

	// New movemnt from Component 

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void MoveComp(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void TurnComp(float Amount);

	//Server movement RPC

	UFUNCTION(Server, Unreliable)
	void ServerMove(float Amount);

	UFUNCTION(Server, Unreliable)
	void ServerTurn(float Amount);

	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireVSFX();


	UFUNCTION(BlueprintImplementableEvent, Category = "Tank Actions")
	void FireVSFX();

	//Used only for simulated proxies, to corectrly display movment VFX and SFX on client side
	UPROPERTY(Replicated, BlueprintReadWrite)
	float SimTankVelocity = 0.f;

	//Disables or enables turret rotation to cursor
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetTurretRotationToCursorState(bool bInputState);

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AProjectile> ProjectileClass;

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void HealthUpdated();

	//Inherits value from controller in SetTeamSettings, used for damage logic
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	ETeam PawnTeam;

	//Changes DynamicTeamColor in players team color
	UFUNCTION()
	void SetTeamSettings(FLinearColor NewTeamColor, ETeam NewTeam);

	//Updates material color of new team on simProxy and clinet sides
	UFUNCTION(NetMulticast, Reliable)
	void MulticastColorSettings(FLinearColor NewTeamColor);

	virtual void PossessedBy(AController* NewController) override;

	virtual void Tick(float DeltaTime) override;
};