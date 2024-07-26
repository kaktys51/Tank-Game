#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "HealthComponent.h"
#include "TurretPawn.generated.h"


UCLASS()
class PART_3_API ATurretPawn : public APawn
{
	GENERATED_BODY()

public:
	ATurretPawn();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
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

	UFUNCTION()
	void RotateTurret(const FVector& LookAtTarget);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Team Color")
	FLinearColor MaterialColor;

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void PostInitializeComponents() override;
};
