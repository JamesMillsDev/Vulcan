#include "Application.h"

#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>

#include "GameTime.h"
#include "Resources.h"
#include "SimpleInput.h"
#include "Gameplay/Actors/World.h"
#include "Graphics/Renderer.h"
#include "Utility/Config.h"

using namespace Tempest;

using std::runtime_error;

Application* Application::m_instance = nullptr;

Application* Application::GetInstance()
{
	return m_instance;
}

void Application::Quit()
{
	GetWindow()->m_isOpen = false;
}

GameInstance* Application::GetGameInstance()
{
	return m_instance->m_game;
}

Window* Application::GetWindow()
{
	return m_instance->m_window;
}

Application::Application()
	: m_config{ new Config{ "Engine" } }, m_window{ new Window{ m_config } }, m_game{ nullptr }
{}

Application::~Application()
{
	delete m_game;
	m_game = nullptr;

	delete m_window;
	m_window = nullptr;

	delete m_config;
	m_config = nullptr;
}

EExitCode Application::Run()
{
	// Attempt to open the window, returning fail code if it does not succeed
	try
	{
		m_window->Open();
	}
	catch (runtime_error& e)
	{
		std::cout << e.what() << "\n";
		return EExitCode::WindowFailedToOpen;
	}

	SimpleInput::Create();
	Renderer::Create(m_config, m_window->m_window);

	// Validate the renderer succeeded to initialise
	if (!Renderer::IsValid())
	{
		// It didn't, so shutdown the window and return the error code.
		m_window->Close();
		return EExitCode::RendererFailedToInit;
	}

	Resources::Init(m_config);
	GameTime::Init();

	// Initialise the application and game instance
	Init(Renderer::m_instance->m_vulkan);
	m_game->Init();

	// Continue to loop until the window requests a close
	while (!m_window->ShouldClose())
	{
		GameTime::Tick();

		SimpleInput::Instance()->ClearStatus();
		glfwPollEvents();

		// Tick the application
		Tick();

		// Tick the game and world
		m_game->Tick();
		m_game->GetWorld()->Tick();

		Renderer::Instance()->BeginFrame();

		// Render the application
		PreRender();
		Render();

		// Render the game and world
		m_game->Render();
		m_game->GetWorld()->PreRender();
		m_game->GetWorld()->Render();
		m_game->GetWorld()->PostRender();

		PostRender();

		Renderer::Instance()->EndFrame();
	}

	Renderer::WaitIdle();

	// delete the current world to pre-cleanup
	delete m_game->m_world;

	// Shutdown the game instance and application
	m_game->Shutdown();

	Shutdown();

	// Shutdown all the subsystems and close the window
	SimpleInput::Destroy();
	Resources::Shutdown();
	Renderer::Destroy();

	m_window->Close();

	// Return success as the whole gameplay loop ran successfully.
	return EExitCode::Success;
}

void Application::Init(Vulkan* vulkan)
{}

void Application::PreRender()
{}

void Application::Render()
{}

void Application::PostRender()
{}

void Application::Tick()
{}

void Application::Shutdown()
{}