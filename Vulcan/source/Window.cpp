#include "Window.h"

#include <stdexcept>

#include <GLFW/glfw3.h>

#include "Utility/Config.h"

using std::runtime_error;

Window::Window(Config* config)
	: m_isOpen{ false }, m_window{ nullptr }
{
	m_width = config->Get<int>("Window.Width");
	m_height = config->Get<int>("Window.Height");
	m_title = config->Get<string>("Application.Title");
	m_resizable = config->Get<bool>("Window.Resizable");
}

Window::~Window() = default;

float Window::Width() const
{
	return static_cast<float>(m_width);
}

void Window::SetWidth(const int w)
{
	m_width = w;
}

float Window::Height() const
{
	return static_cast<float>(m_height);
}

void Window::SetHeight(const int h)
{
	m_height = h;
}

bool Window::ShouldClose() const
{
	return glfwWindowShouldClose(m_window) || !m_isOpen;
}

float Window::Aspect() const
{
	return Width() / Height();
}

GLFWwindow* Window::GlfwHandle() const
{
	return m_window;
}

void Window::Open()
{
	if (glfwInit() != GLFW_TRUE)
	{
		throw runtime_error("GLFW failed to initialise!");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, m_resizable ? GLFW_TRUE : GLFW_FALSE);

	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

	if (m_window == nullptr)
	{
		glfwTerminate();
		throw runtime_error("GLFW failed to create window!");
	}

	m_isOpen = true;
}

void Window::Close()
{
	glfwDestroyWindow(m_window);
	m_window = nullptr;

	glfwTerminate();

	m_isOpen = false;
}