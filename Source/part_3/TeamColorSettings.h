#pragma once

#include "CoreMinimal.h"
#include "PlayerTeams_Enum.h"
#include "TeamColorSettings.generated.h"

USTRUCT(BlueprintType)
struct FTeamColors
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeam Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FLinearColor TeamColor;
};