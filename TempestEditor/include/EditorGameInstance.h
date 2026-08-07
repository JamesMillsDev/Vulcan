#pragma once
#include "Gameplay/GameInstance.h"

namespace Tempest::Editor
{
	// This is just to handle the interface and is completely unused.
	class EditorGameInstance : public GameInstance
	{
	public:
		void Init() override{}
		void Shutdown() override{}
		void Tick() override{}
		void Render() override{}

	};
}
