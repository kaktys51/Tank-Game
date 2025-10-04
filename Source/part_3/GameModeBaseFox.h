
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<ATankPawn> TankPawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	bool bIsPreGameState = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FString SpawnTagGreenTeam = TEXT("Green0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FString SpawnTagBlueTeam = TEXT("Blue0");

private:

	FTimerHandle RestartTimerHandle;

	void RestartGameWithTimer(float Delay);

	void RestartGame();

public:

	UFUNCTION()
	void AddScore();

	UFUNCTION()
	void LoseGame();

	//Called whenever game score is changed
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode Events")
	void ScoreUpdate();

	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	int32 GetCurrentScore();

	//Maximum score needed for win
	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	int32 GetWinScore();


	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode Events")
	void GameLose();

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode Events")
	void GameWin();

	UFUNCTION(BlueprintImplementableEvent, Category = "Pre Game")
	void CheckAllPlayersReady();

	UFUNCTION(BlueprintCallable, Category = "Game Start")
	void SpawnActorsByTeam(const TArray<APlayerController*>& PlayerControllers);

};