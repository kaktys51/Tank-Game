
#include "GameModeBaseFox.h"
#include "TankPawn.h"
#include <Kismet/GameplayStatics.h>
#include "TimerManager.h"

AGameModeBaseFox::AGameModeBaseFox()
{
	DefaultPawnClass = nullptr;

	RestartDelay = 5;
	WinScore = 1;
	CurrentScore = 0;
}

void AGameModeBaseFox::RestartGameWithTimer(float Delay)
{
	UE_LOG(LogTemp, Warning, TEXT("Game restarting soon!"));
	GetWorldTimerManager().SetTimer(RestartTimerHandle, this, &AGameModeBaseFox::RestartGame, Delay, false);
}

void AGameModeBaseFox::RestartGame()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevelName = World->GetMapName();
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
	}
}

void AGameModeBaseFox::AddScore(int32 Amount)
{
	CurrentScore += Amount;

	if (CurrentScore >= WinScore)
	{
		UE_LOG(LogTemp, Warning, TEXT("You won!"));

		GameWin();
		RestartGameWithTimer(RestartDelay);
	}
}

void AGameModeBaseFox::LoseGame()
{
	UE_LOG(LogTemp, Warning, TEXT("You lose!"));

	GameLose();
	RestartGameWithTimer(RestartDelay);
}