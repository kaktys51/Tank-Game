
#include "GameStateFox.h"

void AGameStateFox::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	PlayerStateArrayUpdated();
}

void AGameStateFox::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	PlayerStateArrayUpdated();
}
