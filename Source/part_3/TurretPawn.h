#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
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
	TObjectPtr<USceneComponent> ProjectileSpawnPoint;

	virtual void Tick(float DeltaTime) override;
};
