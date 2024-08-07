
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeBaseFox.generated.h"

class ATankPawn;

UCLASS()
class PART_3_API AGameModeBaseFox : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGameModeBaseFox();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode Settings")
	float RestartDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	int32 WinScore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
	int32 CurrentScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	int32 ScorePerKill;

private:

	FTimerHandle RestartTimerHandle;

	void RestartGameWithTimer(float Delay);

	void RestartGame();

public:

	UFUNCTION()
	void AddScore();

	UFUNCTION()
	void LoseGame();

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode Events")
	void GameLose();

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode Events")
	void GameWin();
};