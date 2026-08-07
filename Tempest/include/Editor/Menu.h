#pragma once

#include <functional>
#include <stack>
#include <string>
#include <vector>

#include "Utility/Collections/TList.h"
#include "Utility/Collections/TStack.h"

using std::stack;
using std::string;
using std::vector;

using MenuOperation = std::function<void()>;

namespace Tempest
{
	class Menu
	{
	private:
		string m_label;
		MenuOperation m_operation;
		vector<Menu*> m_children;

	public:
		Menu();
		explicit Menu(const string& title);
		explicit Menu(const string& label, const MenuOperation& operation);

		~Menu();

	public:
		void Render();

		void AddChild(Menu* child);

	};

	class MenuBuilder
	{
	private:
		struct MenuItemInformation
		{
			string name;
			MenuOperation operation = nullptr;
			vector<MenuItemInformation> children = {};
		};

	private:
		static Menu* GenerateFrom(const MenuItemInformation& info);

	private:
		stack<MenuItemInformation> m_informationStack;
		vector<MenuItemInformation> m_topLevelInfo;

	public:
		Menu* Build();

		MenuBuilder& SubMenu(const string& title);
		MenuBuilder& Item(const string& label, const MenuOperation& operation);
		MenuBuilder& Separator();
		MenuBuilder& End();

	};
}