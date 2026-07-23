#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "Utility/Collections/TList.h"

using std::string;

class Vulkan;

class Shader
{
private:
	static TList<char> ReadShaderFile(const string& fileName);

private:
	string m_path;

	VkShaderModule m_shaderModule;

public:
	explicit Shader(string path);
	~Shader();

public:
	[[nodiscard]] const VkShaderModule& GetShaderModule() const;

private:
	void Init(const Vulkan* vulkan);
	void Destroy();

};