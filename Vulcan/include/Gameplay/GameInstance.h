#pragma once

class Renderer;
class World;

/**
 * @brief Abstract base class for the game logic layer.
 *
 * GameInstance defines the interface that Application calls during the
 * main loop. Derive from this class and implement all four virtual methods
 * to provide game-specific behaviour. The derived type is passed as a
 * template argument to Application::Open(), which constructs and owns
 * the instance for the duration of the session.
 */
class GameInstance
{
	friend class Application;

protected:
	/** @brief The world the Game is currently using */
	World* m_world;

public:
	/** @brief Constructs the GameInstance and creates the root Actor. */
	GameInstance();

	/** @brief Virtual destructor to ensure correct cleanup of derived types. */
	virtual ~GameInstance();

public:
	/**
	 * @brief Called once before the main loop begins.
	 *
	 * Use this to load assets, set up scene state, register systems,
	 * or perform any other one-time startup work.
	 */
	virtual void Init() = 0;

	/**
	 * @brief Called once after the main loop exits.
	 *
	 * Use this to release resources, save state, or perform any other
	 * cleanup that must happen before the application terminates.
	 */
	virtual void Shutdown() = 0;

	/**
	 * @brief Called once per frame to advance game logic.
	 *
	 * All simulation, input handling, and non-rendering updates should
	 * be performed here. Render() is called immediately after.
	 */
	virtual void Tick() = 0;

	/**
	 * @brief Called once per frame to submit draw commands.
	 *
	 * Invoked by Application between BeginFrame() and EndFrame(), so
	 * the raylib drawing context is already active when this is called.
	 * All rendering should be performed here rather than in Tick().
	 */
	virtual void Render() = 0;

	/**
	 * @brief Gets the current world the game is using.
	 * @return The current world.
	 */
	World* GetWorld() const;

};