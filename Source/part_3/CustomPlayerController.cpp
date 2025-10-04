

#include "CustomPlayerController.h"
#include "TankPawn.h"
#include "GameModeBaseFox.h"
#include "PlayerStateFox.h"

ACustomPlayerController::ACustomPlayerController()
{
	PlayerTeam = ETeam::Blue;
}

void ACustomPlayerController::SetPlayerEnabledState(bool bPlayerEnabled)
{
	if (bPlayerEnabled)
	{
		if (GetPawn())
		{
			GetPawn()->EnableInput(this);
		}
	}
	else
	{
		if (GetPawn())
		{
			GetPawn()->DisableInput(this);
		}
	}
}

FLinearColor ACustomPlayerController::GetTeamColor(ETeam ControllerTeam)const
{
	if (!TeamColorsDataAsset) return FLinearColor::White;

	for (const FTeamColors& DataAsset : TeamColorsDataAsset->TeamsAndColors)
	{
		if (DataAsset.Team == ControllerTeam)
		{
			return DataAsset.TeamColor;
		}
	}

	return FLinearColor::White;
}


void ACustomPlayerController::SwitchTeam(int32 NewTeam)
{
	if (!HasAuthority())
	{
		ServerSwitchTeam(NewTeam);
		return;
	}
	PlayerTeam = static_cast<ETeam>(NewTeam);

	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
	{
		ATankPawn* PlayerTank = Cast<ATankPawn>(ControlledPawn);

		if (PlayerTank)
		{
			SetPawnTeam(PlayerTank);
		}
	}
	//Changing Team in PlayerState
	if (APlayerStateFox* CustomPS = GetPlayerState<APlayerStateFox>())
	{
		CustomPS->ServerSetTeam(NewTeam);
	}
}

void ACustomPlayerController::ServerSwitchTeam_Implementation(int32 NewTeam)
{

	PlayerTeam = static_cast<ETeam>(NewTeam);

	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
	{
		ATankPawn* PlayerTank = Cast<ATankPawn>(ControlledPawn);

		if (PlayerTank)
		{
			SetPawnTeam(PlayerTank);
		}
	}

	//Changing Team in PlayerState
	if (APlayerStateFox* CustomPS = GetPlayerState<APlayerStateFox>())
	{
		CustomPS->ServerSetTeam(NewTeam);
	}
}

ETeam ACustomPlayerController::GetPlayerTeam()
{
	return PlayerTeam;
}

void ACustomPlayerController::SetPawnTeam(ATankPawn* PlayerTank)
{

	if (PlayerTank)
	{		
		PlayerTank->SetTeamSettings(GetTeamColor(PlayerTeam), PlayerTeam);
	}
}

void ACustomPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (APlayerStateFox* MyState = GetPlayerState<APlayerStateFox>())
	{
		RecivedPlayerState(MyState);
	}
}

void ACustomPlayerController::ServerSetReady_Implementation(bool bNewReady)
{
	//bIsReady = bNewReady;
	if (APlayerStateFox* CustomPS = GetPlayerState<APlayerStateFox>())
	{
		CustomPS->ServerSetReady(bNewReady);
		GetWorld()->GetAuthGameMode<AGameModeBaseFox>()->CheckAllPlayersReady();
	}	
}
