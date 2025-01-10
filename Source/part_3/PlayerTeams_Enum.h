
#pragma once

#include "CoreMinimal.h"
#include "PlayerTeams_Enum.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Blue     UMETA(DisplayName = "Blue Team"),
	Green    UMETA(DisplayName = "Green Team"),
	Pink     UMETA(DisplayName = "Pink Team"),
	Yellow   UMETA(DisplayName = "Yellow Team")
};
