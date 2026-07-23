#pragma once

#include <chrono>

using std::chrono::high_resolution_clock;

class GameTime final
{
	friend class Application;

private:
	/** @brief The time the application started. Used to calculate the total time. */
	static high_resolution_clock::time_point m_startTime;
	/** @brief The last time the GameTime class ticked. */
	static high_resolution_clock::time_point m_lastTime;
	/** @brief The current frame time. */
	static float m_deltaTime;
	/** @brief The scalar of @c m_deltaTime. Always clamped between @c 0 - @c 1. */
	static float m_timeScale;

public:
	/** @brief Returns a delta time value scaled by @c m_timeScale. */
	static float DeltaTime();

	/** @brief Returns an unscaled delta time. */
	static float UnscaledDeltaTime();
	
	/** @brief Returns the total run time of the application. */
	static float Time();
	
	/** @brief Returns the current timescale of the application. */
	static float TimeScale();

	/** 
	 * @brief Sets the timescale to the newly passed value.
	 * @param ts The new timescale value, will be clamped automatically to between @c 0 - @c 1.
	 */
	static void SetTimeScale(float ts);

private:
	/** @brief Sets the initial values of the game time system. */
	static void Init();

	/** @brief Updates the internal values of the game time system, calculating delta time. */
	static void Tick();


};