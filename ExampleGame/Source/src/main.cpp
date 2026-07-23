#include <Application.h>
#include "DungeonGameInstance.h"

int main()
{
	return static_cast<int>(Vulcan::Application::Open<DungeonGameInstance>());
}
