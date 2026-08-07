#include "Editor/Menu.h"

#include <iostream>

using namespace Tempest;

Menu::Menu() = default;

Menu::Menu(const string& title)
	: m_label{ title }
{

}

Menu::Menu(const string& label, const MenuOperation& operation)
	: m_label{ label }, m_operation{ operation }
{}

Menu::~Menu()
{
	for (auto& child : m_children)
	{
		delete child;
	}
}

void Menu::Render()
{
	std::cout << m_label << "\n";
	for (auto& child : m_children)
	{
		std::cout << "\t";
		child->Render();
	}
}

void Menu::AddChild(Menu* child)
{
	m_children.emplace_back(child);
}

Menu* MenuBuilder::GenerateFrom(const MenuItemInformation& info)
{
	if (info.children.empty())
	{
		return new Menu{ info.name, info.operation };
	}

	Menu* menu = new Menu{ info.name };
	for (const MenuItemInformation& child : info.children)
	{
		menu->AddChild(GenerateFrom(child));
	}

	return menu;
}

Menu* MenuBuilder::Build()
{
	Menu* menu = new Menu;
	for (auto& item : m_topLevelInfo)
	{
		menu->AddChild(GenerateFrom(item));
	}

	return menu;
}

MenuBuilder& MenuBuilder::SubMenu(const string& title)
{
	MenuItemInformation info
	{
		.name = title
	};

	m_informationStack.push(info);
	return *this;
}

MenuBuilder& MenuBuilder::Item(const string& label, const MenuOperation& operation)
{
	MenuItemInformation info
	{
		.name = label,
		.operation = operation
	};

	m_informationStack.top().children.emplace_back(info);
	return *this;
}

MenuBuilder& MenuBuilder::Separator()
{
	m_informationStack.top().children.emplace_back("-----");
	return *this;
}

MenuBuilder& MenuBuilder::End()
{
	MenuItemInformation topLevel = m_informationStack.top();
	m_informationStack.pop();
	if (m_informationStack.empty())
	{
		m_topLevelInfo.emplace_back(topLevel);
	}
	else
	{
		m_informationStack.top().children.emplace_back(topLevel);
	}

	return *this;
}
