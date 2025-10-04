#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TeamColorsDataAsset.h"
#include "PlayerStateFox.generated.h"


UCLASS()
class PART_3_API APlayerStateFox : public APlayerState
{
	GENERATED_BODY()


public:

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeam StatePlayerTeam;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PreGame")
	bool bIsReady = false;

	UFUNCTION(BlueprintCallable)
	ETeam GetPlayerTeamState();

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerSetTeam(int32 NewTeam);
};
