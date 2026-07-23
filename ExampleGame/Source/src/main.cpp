#include <Application.h>
#include "ExampleGameInstance.h"

int main()
{
	return static_cast<int>(Vulcan::Application::Open<ExampleGameInstance>());
}
