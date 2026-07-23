#include "SimpleInput.h"

#include <GLFW/glfw3.h>

#include "Application.h"

SimpleInput* SimpleInput::m_instance = nullptr;

SimpleInput* SimpleInput::Instance()
{
	return m_instance;
}

bool SimpleInput::IsKeyDown(int inputKeyID) const
{
	return m_currentKeys[inputKeyID] == GLFW_PRESS;
}

bool SimpleInput::IsKeyUp(int inputKeyID) const
{
	return m_currentKeys[inputKeyID] == GLFW_RELEASE;
}

bool SimpleInput::WasKeyPressed(int inputKeyID) const
{
	return m_currentKeys[inputKeyID] == GLFW_PRESS && 
		m_lastKeys[inputKeyID] == GLFW_RELEASE;
}

bool SimpleInput::WasKeyReleased(int inputKeyID) const
{
	return m_currentKeys[inputKeyID] == GLFW_RELEASE && 
		m_lastKeys[inputKeyID] == GLFW_PRESS;
}

const TList<int>& SimpleInput::GetPressedKeys() const
{
	return m_pressedKeys;
}

const TList<unsigned int>& SimpleInput::GetPressedCharacters() const
{
	return m_pressedCharacters;
}

bool SimpleInput::IsMouseButtonDown(int inputMouseID) const
{
	return m_currentButtons[inputMouseID] == GLFW_PRESS;
}

bool SimpleInput::IsMouseButtonUp(int inputMouseID) const
{
	return m_currentButtons[inputMouseID] == GLFW_RELEASE;
}

bool SimpleInput::WasMouseButtonPressed(int inputMouseID) const
{
	return m_currentButtons[inputMouseID] == GLFW_PRESS &&
		m_lastButtons[inputMouseID] == GLFW_RELEASE;
}

bool SimpleInput::WasMouseButtonReleased(int inputMouseID) const
{
	return m_currentButtons[inputMouseID] == GLFW_RELEASE &&
		m_lastButtons[inputMouseID] == GLFW_PRESS;
}

float SimpleInput::GetMouseX() const
{
	return m_mouseX;
}

float SimpleInput::GetMouseY() const
{
	return m_mouseY;
}

void SimpleInput::GetMouseXY(float* x, float* y) const
{
	if (x != nullptr)
	{
		*x = m_mouseX;
	}

	if (y != nullptr)
	{
		*y = m_mouseY;
	}
}

float SimpleInput::GetMouseDeltaX() const
{
	return m_mouseX - m_oldMouseX;
}

float SimpleInput::GetMouseDeltaY() const
{
	return m_mouseY - m_oldMouseY;
}

void SimpleInput::GetMouseDelta(float* x, float* y) const
{
	if (x != nullptr)
	{
		*x = m_mouseX - m_oldMouseX;
	}

	if (y != nullptr)
	{
		*y = m_mouseY - m_oldMouseY;
	}
}

float SimpleInput::GetMouseScroll() const
{
	return m_mouseScroll;
}

void SimpleInput::AttachKeyObserver(const KeyCallback& callback)
{
	m_keyCallbacks.Add(callback);
}

void SimpleInput::AttachCharObserver(const CharCallback& callback)
{
	m_charCallbacks.Add(callback);
}

void SimpleInput::AttachMouseButtonObserver(const MouseButtonCallback& callback)
{
	m_mouseButtonCallbacks.Add(callback);
}

void SimpleInput::AttachMouseMoveObserver(const MouseMoveCallback& callback)
{
	m_mouseMoveCallbacks.Add(callback);
}

void SimpleInput::AttachMouseScrollObserver(const MouseScrollCallback& callback)
{
	m_mouseScrollCallbacks.Add(callback);
}

void SimpleInput::Create()
{
	m_instance = new SimpleInput;
}

void SimpleInput::Destroy()
{
	delete m_instance;
}

void SimpleInput::ClearStatus()
{
	m_pressedCharacters.Clear();

	GLFWwindow* window = Application::GetWindow()->GlfwHandle();

	m_pressedKeys.Clear();

	// Update keys
	for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i)
	{
		m_lastKeys[i] = m_currentKeys[i];

		if ((m_currentKeys[i] = glfwGetKey(window, i)) == GLFW_PRESS)
		{
			m_pressedKeys.Add(m_currentKeys[i]);
		}
	}

	// Update mouse
	for (int i = 0; i < 8; ++i)
	{
		m_lastButtons[i] = m_currentButtons[i];
		m_currentButtons[i] = glfwGetMouseButton(window, i);
	}

	// Update old mouse position
	m_oldMouseX = m_mouseX;
	m_oldMouseY = m_mouseY;
}

void SimpleInput::OnMouseMove(int newXPos, int newYPos)
{
	m_mouseX = static_cast<float>(newXPos);
	m_mouseY = static_cast<float>(newYPos);

	if (m_firstMouseMove)
	{
		// On first move after Startup/entering window reset old mouse position
		m_oldMouseX = static_cast<float>(newXPos);
		m_oldMouseY = static_cast<float>(newYPos);
		m_firstMouseMove = false;
	}
}

SimpleInput::SimpleInput()
	: m_oldMouseX{ 0 }, m_oldMouseY{ 0 }, m_firstMouseMove{ false },
	m_lastKeys{ new int[GLFW_KEY_LAST + 1] }, m_currentKeys{ new int[GLFW_KEY_LAST + 1] }
{
	GLFWwindow* window = Application::GetWindow()->GlfwHandle();

	for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i)
	{
		m_lastKeys[i] = m_currentKeys[i] = glfwGetKey(window, i);
	}

	for (int i = 0; i < 8; ++i)
	{
		m_lastButtons[i] = m_currentButtons[i] = glfwGetMouseButton(window, i);
	}

	// set up callbacks
	auto KeyPressCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			for (auto& f : m_instance->m_keyCallbacks)
			{
				f(window, key, scancode, action, mods);
			}
		};

	auto CharacterInputCallback = [](GLFWwindow* window, unsigned int character)
		{
			Instance()->m_pressedCharacters.Add(character);

			for (auto& f : m_instance->m_charCallbacks)
			{
				f(window, character);
			}
		};

	auto MouseMoveCallback = [](GLFWwindow* window, double x, double y)
		{
			int w = 0, h = 0;
			glfwGetWindowSize(window, &w, &h);

			Instance()->OnMouseMove(static_cast<int>(x), h - static_cast<int>(y));

			for (auto& f : m_instance->m_mouseMoveCallbacks)
			{
				f(window, x, h - y);
			}
		};

	auto MouseInputCallback = [](GLFWwindow* window, int button, int action, int mods)
		{
			for (auto& f : m_instance->m_mouseButtonCallbacks)
			{
				f(window, button, action, mods);
			}
		};

	auto MouseScrollCallback = [](GLFWwindow* window, double xOffset, double yOffset)
		{
			m_instance->m_mouseScroll += static_cast<float>(yOffset);

			for (auto& f : m_instance->m_mouseScrollCallbacks)
			{
				f(window, xOffset, yOffset);
			}
		};

	auto MouseEnterCallback = [](GLFWwindow* window, int entered)
		{
			// Set flag to prevent large mouse delta on entering screen
			m_instance->m_firstMouseMove = true;
		};

	glfwSetKeyCallback(window, KeyPressCallback);
	glfwSetCharCallback(window, CharacterInputCallback);
	glfwSetMouseButtonCallback(window, MouseInputCallback);
	glfwSetCursorPosCallback(window, MouseMoveCallback);
	glfwSetScrollCallback(window, MouseScrollCallback);
	glfwSetCursorEnterCallback(window, MouseEnterCallback);

	m_mouseX = 0;
	m_mouseY = 0;
	m_mouseScroll = 0;
}

SimpleInput::~SimpleInput()
{
	delete[] m_lastKeys;
	delete[] m_currentKeys;
}