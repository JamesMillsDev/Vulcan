#include "Editor/Menu.h"

#include <imgui.h>
#include <iostream>

using namespace Tempest::Editor;

Menu::Menu(string title)
	: m_label{ std::move(title) }, m_isRoot{ false }, m_isSeparator{ false }
{

}

Menu::Menu(string label, MenuOperation operation)
	: m_label{ std::move(label) }, m_operation{ std::move(operation) }, m_isRoot{ false }, m_isSeparator{ false }
{}

Menu::~Menu()
{
	for (const Menu* child : m_children)
	{
		delete child;
	}
}

Menu::Menu()
	: m_isRoot{ true }, m_isSeparator{ false }
{}

void Menu::Render() const
{
	if (m_isRoot)
	{
		ImGui::BeginMainMenuBar();

		for (const Menu* menu : m_children)
		{
			menu->Render();
		}

		ImGui::EndMainMenuBar();
	}
	// This is a leaf node
	else if (m_children.empty())
	{
		if (m_isSeparator)
		{
			ImGui::Separator();
		}
		else
		{
			ImGui::PushID(m_label.c_str());
			if (ImGui::MenuItem(m_label.c_str()))
			{
				m_operation();
			}
			ImGui::PopID();
		}
	}
	else if (ImGui::BeginMenu(m_label.c_str()))
	{
		for (const Menu* menu : m_children)
		{
			menu->Render();
		}

		ImGui::EndMenu();
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
		Menu* menu = new Menu{ info.name, info.operation };
		menu->m_isSeparator = info.isSeparator;

		return menu;
	}

	Menu* menu = new Menu{ info.name };
	for (const MenuItemInformation& child : info.children)
	{
		menu->AddChild(GenerateFrom(child));
	}

	return menu;
}

Menu* MenuBuilder::Build() const
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
		.operation = operation,
		.isSeparator = false
	};

	m_informationStack.top().children.emplace_back(info);
	return *this;
}

MenuBuilder& MenuBuilder::Separator()
{
	m_informationStack.top().children.emplace_back(MenuItemInformation{ .name = "-----", .isSeparator = true });
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
