
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "DestroyedPawn.generated.h"

UCLASS()
class PART_3_API ADestroyedPawn : public APawn
{
	GENERATED_BODY()

public:
	ADestroyedPawn();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> TurretMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USceneComponent> MainExplosionLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VFX")
	float OffsetMainExplosionLocation;

	//Sets location for vfx closer to player camera, to prevent mesh overlaping
	void SetMainExplosionLocation();

	//Offsets turret location on z axis after spawn, to prevent collision issues 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawn")
	float TurretOffset;

	void SetTurretPosition(const FTransform& TurretTransform);
	void SetBasePosition(const FTransform& BaseTransform);

protected:
	virtual void BeginPlay() override;

};
