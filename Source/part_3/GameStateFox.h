
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameStateFox.generated.h"


UCLASS()
class PART_3_API AGameStateFox : public AGameStateBase
{
	GENERATED_BODY()
	

public:

	UFUNCTION(BlueprintImplementableEvent)
	void PlayerStateArrayUpdated();

protected:

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

};
