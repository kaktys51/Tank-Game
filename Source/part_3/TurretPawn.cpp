
#include "TurretPawn.h"

ATurretPawn::ATurretPawn()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ATurretPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATurretPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATurretPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}