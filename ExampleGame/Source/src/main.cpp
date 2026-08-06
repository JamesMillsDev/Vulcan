#include <Application.h>
#include "ExampleGameInstance.h"

int main()
{
	return static_cast<int>(Fulk::Application::Open<ExampleGameInstance>());
}
