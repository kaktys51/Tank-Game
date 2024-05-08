
#include "TurretPawn.h"

ATurretPawn::ATurretPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(CapsuleComponent);

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(CapsuleComponent);

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPoint->SetupAttachment(CapsuleComponent);

	MaterialColor = FLinearColor(1.0f, 1.0f, 1.0f);
}

void ATurretPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

TArray<FString> ATurretPawn::GetMaterialParametrs() const
{
	TArray<FString> LocalMaterialParameters;

	LocalMaterialParameters.Add("TeamColor");

	return LocalMaterialParameters;
}

TArray<FName> ATurretPawn::GetSlotNames() const
{ 
	TArray<FName> BaseMaterialSlotNames;

	if (BaseMesh)
	{
		BaseMaterialSlotNames = BaseMesh->GetMaterialSlotNames();
	}

	return BaseMaterialSlotNames;
}



void ATurretPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATurretPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UStaticMeshComponent* StaticBaseMash = BaseMesh;
	int32 SlotIndex = StaticBaseMash->GetMaterialIndex(MaterialSlotName);

	if (SlotIndex != INDEX_NONE)
	{
		UMaterialInterface* MaterialInterface = StaticBaseMash->GetMaterial(SlotIndex);
		if (MaterialInterface)
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialInterface, StaticBaseMash);

			FName MaterialParametrName = MaterialParametrs;
			FLinearColor NewParametrColor = MaterialColor;
			DynamicMaterial->SetVectorParameterValue(MaterialParametrName, NewParametrColor);

			BaseMesh->SetMaterial(SlotIndex, DynamicMaterial);
			TurretMesh->SetMaterial(SlotIndex, DynamicMaterial);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Material not found"));
	}
}