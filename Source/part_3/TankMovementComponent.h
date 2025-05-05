
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TankMovementComponent.generated.h"

class ATankPawn;

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

	void SimulatedProxyMovement(float DeltaTime);

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

	// Saves the last processed timestamp from a client move to prevent confirming moves that arrive out of order
	UPROPERTY()
	float LastProcessedMoveTimeStamp = 0.f;


	UFUNCTION(Server, Unreliable)
	void Server_Move(const FTankSafeMove& MoveData);

	void CheckMovementError(const FTankSafeMove& MoveData);

	UFUNCTION(NetMulticast, BlueprintCallable, Reliable)
	void Multicast_CorrectionData(const FTransform& ServerTransform, float TimeStamp);

	// Utility value, used for smoothing correction
	float CurrentCorrectionDistance = 0.f;

	// Used by server, as an error tolerance between cilent predicted move and server move
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Network")
	float DistanceCorrection = 20.f;

	// Used by server, as an error tolerance between cilent predicted turn and server move
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Network", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float TurnCorrection = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TurnSpeed = 65.f;

	UFUNCTION(BlueprintCallable, Category = "Tank Actions")
	void Turn(float Amount);

	//************************************
	//
	// Network smoothing tests variables

	// Determines speed of smoothing interpolatinon (visual)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float SmoothingSpeed = 10.f;

	bool bIsVisualCorrectionActive = false;


	// **********************

	FVector BodyVisualLocation;
	FQuat BodyVisualRotation;

	float LastCorrectionTime = 0.f;

	// Specifies how often the server can send correction RPCs.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network|Replication")
	float CorrectionCooldown = 0.5f;

	// Changes apha curveture for EaseOut correction interpolation 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	float CrorrectAphaCurveture = 1.5f;

	float CorrectInterpTimeElapsded = 0.f;

	// Time of correction interpolation 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	float CorrectInterpDuration = 0.1f; 

	// Minimal time for correction duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	float MinDuration = 0.1f;
	// Max time for correction duratuion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	float MaxDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	float DistanceThresholdLow = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	float DistanceThresholdHigh = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	bool bIsUpdateVisualActive = false;

	bool bIsCorrectionActive = false;

	bool bIsCoorectionOngoing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Visual")
	float VisualInterpolationSpeed = 6.f;

	// Update visual location of actor
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void UpdateVisual(float DeltaTime);

	// Trace depth for aligning of tank to surface
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
	float TraceDepth = 200.f;

	// Gravity Linetrace leght = is Capsule Half Height + margin. May be increased for mounted terrains 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
	float GravityLineTraseMargin = 10.f;

	// Determines speed of surface alignment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
	float AlligmentSpeed = 3.f;

	// Contains all points for line trace from owner class
	TArray<USceneComponent*> Samples;

	FQuat AlignVisualRootToGround();

	void SimpleGravity(float DeltaTime);
};