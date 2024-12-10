

#include "CustomPlayerController.h"
#include "TankPawn.h"

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

ETeam ACustomPlayerController::GetPlayerTeam()
{
	return PlayerTeam;
}

void ACustomPlayerController::SetPawnTeam(ATankPawn* PlayerTank)
{
	//APawn* ControlledPawn = ACustomPlayerController::GetPawn();

	if (PlayerTank)
	{
		//ATankPawn* PlayerTank = Cast<ATankPawn>(ControlledPawn);
		
		switch (PlayerTeam)
		{
		case ETeam::Blue:
			PlayerTank->SetTeamSettings(FLinearColor(0.0f, 0.0f, 1.0f), PlayerTeam);
			break;
		case ETeam::Green:
			PlayerTank->SetTeamSettings(FLinearColor(0.0f, 1.0f, 0.0f), PlayerTeam);
			break;
		case ETeam::Pink:
			PlayerTank->SetTeamSettings(FLinearColor(1.0f, 0.75f, 0.8f), PlayerTeam);
			break;
		case ETeam::Yellow:
			PlayerTank->SetTeamSettings(FLinearColor(1.0f, 1.0f, 0.0f), PlayerTeam);
			break;
		default:
			PlayerTank->SetTeamSettings(FLinearColor(0.0f, 0.0f, 1.0f), PlayerTeam);
			break;
		}
	}
}
