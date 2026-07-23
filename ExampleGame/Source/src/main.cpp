#include <Application.h>
#include "DungeonGameInstance.h"

int main()
{
	return static_cast<int>(Application::Open<DungeonGameInstance>());
}
