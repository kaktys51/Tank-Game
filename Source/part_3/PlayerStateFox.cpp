
#include "PlayerStateFox.h"
#include "Net/UnrealNetwork.h"
#include "GameModeBaseFox.h"

void APlayerStateFox::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerStateFox, bIsReady);
	DOREPLIFETIME(APlayerStateFox, StatePlayerTeam);
}

void APlayerStateFox::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;
	GetWorld()->GetAuthGameMode<AGameModeBaseFox>()->CheckAllPlayersReady();
}

void APlayerStateFox::ServerSetTeam_Implementation(int32 NewTeam)
{
	StatePlayerTeam = static_cast<ETeam>(NewTeam);
}

ETeam APlayerStateFox::GetPlayerTeamState()
{
	return StatePlayerTeam;
}
