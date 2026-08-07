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

namespace Tempest::Editor
{
	class Menu
	{
		friend class MenuBuilder;

	private:
		string m_label;
		MenuOperation m_operation;
		vector<Menu*> m_children;
		bool m_isRoot;
		bool m_isSeparator;

	public:
		explicit Menu(string title);
		explicit Menu(string label, MenuOperation operation);

		~Menu();

	private:
		Menu();

	public:
		void Render() const;

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
			bool isSeparator = false;
		};

	private:
		static Menu* GenerateFrom(const MenuItemInformation& info);

	private:
		stack<MenuItemInformation> m_informationStack;
		vector<MenuItemInformation> m_topLevelInfo;

	public:
		Menu* Build() const;

		MenuBuilder& SubMenu(const string& title);
		MenuBuilder& Item(const string& label, const MenuOperation& operation);
		MenuBuilder& Separator();
		MenuBuilder& End();

	};
}