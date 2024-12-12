

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

void ACustomPlayerController::SwichTeam(int32 NewTeam)
{
	if (!HasAuthority())
	{
		ServerSwichTeam(NewTeam);
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
}

void ACustomPlayerController::ServerSwichTeam_Implementation(int32 NewTeam)
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
			PlayerTank->SetTeamSettings(FLinearColor(1.0f, 0.1f, 0.65f), PlayerTeam);
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
