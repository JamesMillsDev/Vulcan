#pragma once

#include <string>

#include <GLFW/glfw3.h>

using std::string;

struct GLFWwindow;

class Config;

class Window
{
	friend class Application;
	friend class Vulkan;

private:
	int m_width;
	int m_height;
	string m_title;
	bool m_resizable;
	bool m_isOpen;

	GLFWwindow* m_window;

private:
	explicit Window(Config* config);
	~Window();

public:
	[[nodiscard]] float Width() const;

	void SetWidth(int w);

	[[nodiscard]] float Height() const;

	void SetHeight(int h);

	[[nodiscard]] bool ShouldClose() const;

	[[nodiscard]] float Aspect() const;

	[[nodiscard]] GLFWwindow* GlfwHandle() const;

private:
	void Open();

	void Close();

};