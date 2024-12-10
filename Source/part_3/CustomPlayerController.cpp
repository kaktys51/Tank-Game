

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
			PlayerTank->MaterialColor = FLinearColor(0.0f, 0.0f, 1.0f);
			PlayerTank->SetTeamSettings();
			break;
		case ETeam::Green:
			PlayerTank->MaterialColor = FLinearColor(0.0f, 1.0f, 0.0f);
			break;
		case ETeam::Pink:
			PlayerTank->MaterialColor = FLinearColor(1.0f, 0.75f, 0.8f);
			break;
		case ETeam::Yellow:
			PlayerTank->MaterialColor = FLinearColor(1.0f, 1.0f, 0.0f);
			break;
		default:
			PlayerTank->MaterialColor = FLinearColor(0.0f, 0.0f, 1.0f);
			break;
		}
	}
}
