#include "HunterGameMode.h"
#include "Characters/ArinValeCharacter.h"

AHunterGameMode::AHunterGameMode()
{
    DefaultPawnClass = AArinValeCharacter::StaticClass();
}
