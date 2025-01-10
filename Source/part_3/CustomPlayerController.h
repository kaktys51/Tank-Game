#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerTeams_Enum.h"
#include "TeamColorsDataAsset.h"
#include "CustomPlayerController.generated.h"

class ATankPawn;


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

	//Data Asset that contains team colors, setting up in editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	UTeamColorsDataAsset* TeamColorsDataAsset;

	//Extract color from data asset
	UFUNCTION()
	FLinearColor GetTeamColor(ETeam ControllerTeam) const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerEnabledState(bool bPlayerEnabled);

	UFUNCTION(BlueprintCallable)
	ETeam GetPlayerTeam();

	//Used in conscole to change plyaer team
	UFUNCTION(Exec, BlueprintCallable, Category = "Team")
	void SwitchTeam(int32 NewTeam);

	//Used for correct change of teams on client side
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Team")
	void ServerSwitchTeam(int32 NewTeam);

	//applies color for Team Material on tank
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetPawnTeam(ATankPawn* PlayerTank);

};
