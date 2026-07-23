#pragma once

#include <cassert>
#include <string>
#include <type_traits>

#include "Window.h"

#include "Gameplay/GameInstance.h"

#if _DEBUG
#include <crtdbg.h>
#endif

using std::string;

class Config;
class Renderer;

enum class EExitCode : uint8_t
{
	Success,
	WindowFailedToOpen,
	RendererFailedToInit,
};

class Application
{
private:
	/** @brief The singleton Application instance. nullptr when no app is running. */
	static Application* m_instance;

public:
	/**
	 * @brief Creates the application, runs the game loop, then destroys it.
	 *
	 * Asserts that no Application instance already exists, then constructs one,
	 * instantiates @p GAME as the active GameInstance, and enters the main loop
	 * via Run(). The Application and the game instance are destroyed before this
	 * function returns.
	 *
	 * @tparam GAME A concrete type derived from GameInstance to instantiate.
	 *
	 * @return The exit code produced by the game loop.
	 *
	 * @pre  No Application instance is currently alive.
	 * @post The Application singleton is destroyed and m_instance is null.
	 */
	template<typename GAME>
	static EExitCode Open();

	/**
	 * @brief Returns a pointer to the active Application singleton.
	 *
	 * The returned pointer should not be stored beyond immediate use.
	 * The singleton's lifetime is controlled entirely by Open(); holding a
	 * pointer past the end of that call will prevent correct destruction.
	 *
	 * @return A pointer to the current Application, or nullptr if none exists.
	 */
	static Application* GetInstance();

	/** @brief Tells the application to close down safely this frame. */
	static void Quit();

	/** @brief Returns the pointer to the window instance. */
	static Window* GetWindow();

private:
	/** @brief The global engine config for this application. */
	Config* m_config;

	/** @brief The window managed by this application. */
	Window* m_window;

	/** @brief The active game instance managed by this application. */
	GameInstance* m_game;

private:
	Application();
	~Application();

private:
	/**
	 * @brief Executes the main game loop until the game signals an exit.
	 * @return The exit code indicating why the loop terminated.
	 */
	[[nodiscard]] EExitCode Run() const;

};

template <typename GAME>
EExitCode Application::Open()
{
	// Ensure GAME inherits from the GameInstance type
	static_assert(std::is_base_of_v<GameInstance, GAME>, "GAME must derive from GameInstance!");

	// Validate that the open function has not already been called
	assert(m_instance == nullptr && "Cannot create a second instance of application!");

	// If we are in a debug build, enable memory leak detection
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Create an instance of the game and application
	m_instance = new Application;
	m_instance->m_game = new GAME;

	// Run the application, gathering the exit code
	const EExitCode exitCode = m_instance->Run();

	// Clean up the application instance and return the exit code
	delete m_instance;
	return exitCode;
}