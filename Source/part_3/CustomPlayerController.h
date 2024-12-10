#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"

class ATankPawn;

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Blue     UMETA(DisplayName = "Blue Team"),
	Green    UMETA(DisplayName = "Green Team"),
	Pink     UMETA(DisplayName = "Pink Team"),
	Yellow   UMETA(DisplayName = "Yellow Team")
};

UCLASS()
class PART_3_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ACustomPlayerController();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeam PlayerTeam;

public:

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerEnabledState(bool bPlayerEnabled);

	UFUNCTION(BlueprintCallable)
	ETeam GetPlayerTeam();

	//applies color for Team Material on tank
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetPawnTeam(ATankPawn* PlayerTank);

};
