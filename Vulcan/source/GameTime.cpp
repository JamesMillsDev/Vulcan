#include "GameTime.h"

#include "Maths/Maths.h"

using Duration = std::chrono::duration<float>;

high_resolution_clock::time_point GameTime::m_startTime;
high_resolution_clock::time_point GameTime::m_lastTime;
float GameTime::m_deltaTime;
float GameTime::m_timeScale;

float GameTime::DeltaTime()
{
	return m_deltaTime * m_timeScale;
}

float GameTime::UnscaledDeltaTime()
{
	return m_timeScale;
}

float GameTime::Time()
{
	return Duration(m_lastTime - m_startTime).count();
}

float GameTime::TimeScale()
{
	return m_timeScale;
}

void GameTime::SetTimeScale(const float ts)
{
	m_timeScale = Maths::Clamp01(ts);
	m_timeScale = ts;
}

void GameTime::Init()
{
	m_lastTime = high_resolution_clock::now();
	m_startTime = m_lastTime;

	m_timeScale = 1.f;
	m_deltaTime = 0.f;
}

void GameTime::Tick()
{
	const high_resolution_clock::time_point currentTime = high_resolution_clock::now();
	const Duration elapsedTime(currentTime - m_lastTime);

	m_lastTime = currentTime;
	m_deltaTime = elapsedTime.count();
}
