
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

USTRUCT()
struct FTankSafeMove
{
	GENERATED_BODY()

public:
	FTankSafeMove() : InputVector(FVector::ZeroVector), SavedRotation(FRotator::ZeroRotator), SavedLocation(FVector::ZeroVector) {};

	UPROPERTY()
	float TimeStamp = 0.f;

	UPROPERTY()
	float DeltaTime = 0.f;

	UPROPERTY()
	FVector InputVector;

	UPROPERTY()
	float InputRotation = 0.f;

	UPROPERTY()
	FRotator SavedRotation;

	UPROPERTY()
	FVector_NetQuantize100 SavedLocation;

	FTankSafeMove(float inTimeStamp, float inDeltaTime, FVector inInputVector, FVector inTankLocation) : TimeStamp(inTimeStamp), DeltaTime(inDeltaTime), InputVector(inInputVector)
	{
		SavedLocation = inTankLocation;
	}
};

//
// Problem with Net_CurrentMoveAmount
//
USTRUCT()
struct FSaveNetData
{
	GENERATED_BODY()

public:

	FSaveNetData();

	UPROPERTY()
	float TimeStamp;
	UPROPERTY()
	uint8 Net_CurrentMoveAmount;
	UPROPERTY()
	FVector_NetQuantize100 Net_MovedLocation;
	UPROPERTY()
	FQuat Net_Rotation;

	FSaveNetData(float NewTimeStamp, float NewSaved_CurrnetMoveAmont, FVector NewSaved_Location, FRotator NewRotation)
	{
		Net_CurrentMoveAmount = static_cast<uint8>(NewSaved_CurrnetMoveAmont * 255);
		Net_MovedLocation = NewSaved_Location;
		Net_Rotation = FQuat(NewRotation);
	}

};



UCLASS()
class PART_3_API UTankMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:

	UTankMovementComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	APawn* PawnOwner;
	ATankPawn* TankOwner;

	float CurrentTurnAmount = 0.f;

	// Utility values for movement replication in Tick
	bool bIsPreformedMove = false;
	bool bIsPreformedTurn = false;


	// Used for replication of pawn location at simulated proxies
	UPROPERTY(ReplicatedUsing = OnRep_SimProxyTransform)
	FTransform SimProxyTransform;
	
	// Utility bool that prevent using SimProxyTransform until it's receive value from server
	bool bIsSimProxyTransformUpdated = false;

	UFUNCTION()
	void OnRep_SimProxyTransform();

	// Determines interpolation speed of SimProxyTransform (How fast replicated location applied)  
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Replication")
	float SimProxyAlpha = 10.f;

	// Recive and save input vector from pawn
	void AddMoveInputVector(const FVector& WorldVector);
	
	// Recive and save turn input value from pawn
	void AddTurnValue(const float Direction);

	// Saved input vector for movement calculation
	FVector PendingInput;

	// Saved input value for turning calculation
	float PendingTurnInput;

	// Determines a deadzone for turning, which means, lower threshold value that will be perceived as 0
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Turning")
	float DeadZoneTurning = 0.01f;

	// Saved moves for client prediction (correction)
	TArray<FTankSafeMove> SavedMoves;


	UFUNCTION(Server, Unreliable)
	void Server_Move(const FTankSafeMove& MoveData);

	UFUNCTION(NetMulticast, BlueprintCallable, Reliable)
	void Multicast_CorrectionData(const FTransform& ServerTransform, float TimeStamp);


	//Used by server, as an error tolerance between cilent predicted move and server move
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Network")
	float DistanceCorrection = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TurnSpeed = 65.f;

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Turn(float Amount);

protected:

	TArray<FSaveMove> PendingMoves;
};