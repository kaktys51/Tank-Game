#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TeamColorSettings.h"
#include "TeamColorsDataAsset.generated.h"


UCLASS(BlueprintType)
class PART_3_API UTeamColorsDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    TArray<FTeamColors> TeamsAndColors;
};