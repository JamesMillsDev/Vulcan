#include <Application.h>
#include "ExampleGameInstance.h"

int main()
{
	return static_cast<int>(Tempest::Application::Open<ExampleGameInstance>());
}
