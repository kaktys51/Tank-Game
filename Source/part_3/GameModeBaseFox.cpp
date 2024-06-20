
#include "GameModeBaseFox.h"
#include "TankPawn.h"
#include <Kismet/GameplayStatics.h>

AGameModeBaseFox::AGameModeBaseFox()
{
	DefaultPawnClass = ATankPawn::StaticClass();

}



