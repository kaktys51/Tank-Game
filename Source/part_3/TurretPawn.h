#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "HealthComponent.h"
#include "DestroyedPawn.h"
#include "Net/UnrealNetwork.h"
#include "TurretPawn.generated.h"

class ATankPawn;
class ATowerPawn;

UCLASS()
class PART_3_API ATurretPawn : public APawn
{
	GENERATED_BODY()

public:
	ATurretPawn();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Default")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Default")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Default")
	TObjectPtr<UStaticMeshComponent> TurretMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USceneComponent> ProjectileSpawnPointFox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (GetOptions = "GetSlotNames"), Category = "Team Color")
	FName MaterialSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (GetOptions = "GetMaterialParametrs"), Category = "Team Color")
	FName MaterialParametrs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TurretRotationAcceleration = 2.f;

	UFUNCTION()
	TArray<FString> GetMaterialParametrs() const;

	UFUNCTION()
	TArray<FName> GetSlotNames() const;

	UFUNCTION()
	void TurretRotationToCursor();

	UFUNCTION(Server, Unreliable)
	void ServerTurretRotationToCursor();

	UFUNCTION(BlueprintCallable)
	void RotateTurret(const FVector& LookAtTarget);

	UFUNCTION(Server, Unreliable)
	void ServerRotateTurret(const FVector& LookAtTarget);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Team Color")
	FLinearColor MaterialColor;

	//Destroys the original actor and spawns the destroyed version. 
	UFUNCTION()
	void HandleDeath();

	UFUNCTION(Server, Unreliable)
	void ServerHandleDeath();

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Destruction")
	TSubclassOf<ADestroyedPawn> DestroyedClass;

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void PostInitializeComponents() override;
};
