
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TankMovementComponent.generated.h"

class ATankPawn;

USTRUCT()
struct FSaveMove
{
	GENERATED_BODY()

public:

	FSaveMove();

	UPROPERTY()
	float TimeStamp;
	UPROPERTY()
	float Saved_CurrentMoveAmount;
	UPROPERTY()
	FVector Saved_MovedLocation;
	UPROPERTY()
	FVector Saved_Velocity;
	UPROPERTY()
	FRotator Saved_Rotation;

	FSaveMove(float NewTimeStamp, float NewSaved_CurrnetMoveAmont, FVector NewSaved_Location, FVector NewSaved_Velocity, FRotator NewRotation) :
		TimeStamp(NewTimeStamp), Saved_CurrentMoveAmount(NewSaved_CurrnetMoveAmont), Saved_MovedLocation(NewSaved_Location), Saved_Velocity(NewSaved_Velocity), Saved_Rotation(NewRotation)
	{}

};


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

	//Used by server, as an error tolerance between cilent predicted move and server move
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Network")
	float DistanceCorrection = 20.f;

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

	//Movement funcs

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Move(float Amount);

	/*
	 * As client preformed move, he sends data about this move on server.
	 * Server apllyes this data to preform its own move, and then compares its own movement data with client.
	 * If nescessary, calls Multicast_SendCorrectionData
	*/
	UFUNCTION(Server, Unreliable)
	//void Server_SendMove(const FSaveMove MoveData);
	void Server_SendMove(const FSaveMove& MoveData);

	UFUNCTION(NetMulticast, BlueprintCallable, Reliable)
	void Multicast_SendCorrectionData(float TimeStamp, FVector CorrectedLoacation, FVector CorrectedVelocity, FRotator CorrectedRotation);

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Turn(float Amount);

protected:

	TArray<FSaveMove> PendingMoves;
};