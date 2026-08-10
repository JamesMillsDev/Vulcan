#pragma once

#include <string>

using std::string;

namespace Tempest::Editor
{
	class EditorWindow
	{
		friend class EditorApplication;

	private:
		string m_title;

	public:
		explicit EditorWindow(string title);
		virtual ~EditorWindow();

	protected:
		virtual void OnRender() = 0;

	private:
		void Render();

	};
}