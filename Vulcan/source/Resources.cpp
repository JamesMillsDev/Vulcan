// ReSharper disable CppClangTidyBugproneThrowingStaticInitialization

#include "Resources.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <vector>

#include "Utility/Config.h"

using std::ifstream;
using std::vector;

TMap<string, uint32> Resources::m_fileMappings;
TMap<string, ResourceData> Resources::m_resources;
string Resources::m_resourceDir;
string Resources::m_resourceFileName;

namespace 
{
	vector<string> Split(const string& str, const string& delimiter)
	{
		vector<string> tokens;
		size_t start = 0;
		size_t end = str.find(delimiter);

		while (end != std::string::npos)
		{
			tokens.push_back(str.substr(start, end - start));
			start = end + delimiter.length();
			end = str.find(delimiter, start);
		}

		// Add the last remaining token
		tokens.push_back(str.substr(start));
		return tokens;
	}

	void ReplaceAll(string& str, const string& from, const string& to)
	{
		if (from.empty())
		{
			return;
		}

		size_t startPos = 0;
		while ((startPos = str.find(from, startPos)) != string::npos)
		{
			str.replace(startPos, from.length(), to);

			startPos += to.length(); 
		}
	}

	void ScanTo(ifstream& file, const string& id, int& dataLength)
	{
		int currentIdLength = 0;

		while (file.tellg() != ifstream::end)
		{
			file.read(reinterpret_cast<char*>(&currentIdLength), sizeof(int));

			string currentId;
			currentId.resize(currentIdLength);
			file.read(currentId.data(), static_cast<int64>(currentId.length()));

			// Read the next binary data length
			file.read(reinterpret_cast<char*>(&dataLength), sizeof(int));

			// If the current id is not the one we want, skip
			if (currentId != id)
			{
				file.seekg(dataLength, std::ios::cur);
				continue;
			}

			break;
		}
	}
}

ResourceData& Resources::Find(string id)
{
	std::ranges::replace(id, '/', '\\');

	// Only mapped files can be loaded
	assert(m_fileMappings.ContainsKey(id));

	// If the resource is already loaded, return the data
	if (m_resources.ContainsKey(id))
	{
		return *m_resources[id];
	}

	// Attempt to open the correct binary file
	const uint32 resourceFileIndex = *m_fileMappings[id];
	const string path = m_resourceDir + "/" + m_resourceFileName + std::to_string(resourceFileIndex) + ".res";
	ifstream resourceFile(
		path, 
		std::ios::binary
	);

	// Validate if the resource file opened
	ResourceData data { .data = nullptr, .length = 0 };
	if (resourceFile.is_open())
	{
		ScanTo(resourceFile, id, data.length);

		data.data = new uint8[data.length];
		resourceFile.read(reinterpret_cast<int8*>(data.data), data.length);
	}

	// If data was loaded successfully, store it
	if (data.length != 0)
	{
		m_resources.Add(id, data);
	}

	// Close the file and return the loaded data
	resourceFile.close();
	return *m_resources[id];
}

void Resources::Init(Config* config)
{
	assert(config != nullptr);

	// Get the mapping file and the content directory
	m_resourceDir = config->Get<string>("Engine.Content.Directory");
	m_resourceFileName = config->Get<string>("Engine.Content.FileName");
	const string mappingsFilePath = m_resourceDir + "/" + m_resourceFileName + ".mappings";

	// Attempt to open the mappings file
	ifstream mappingsFile(mappingsFilePath);
	if (mappingsFile.is_open())
	{
		// Read the contents of the mapping file
		string line, fileContents;
		while (std::getline(mappingsFile, line))
		{
			fileContents += line;
		}

		// Remove the {} from the file
		line.erase(0, 1);
		line.erase(line.size() - 1, line.size() - 1);

		// Remove every '[' and ' ' from the file
		std::ranges::replace(line, '[', ' ');
		std::erase(line, ' ');

		// Split the map into it's parts by the "]," delimiter
		for (string part : Split(line, "],"))
		{
			// Get the index from the mapping
			int index = part[0] - '0';
			std::erase(part, ']');

			// Iterate over each mapping
			for (string path : Split(part.substr(2), ",'"))
			{
				// Clean up double '\'
				std::erase(path, '\'');
				ReplaceAll(path, "\\\\", "\\");

				// Store the mapping
				m_fileMappings.Add(path, index);
			}
		}
	}

	mappingsFile.close();
}

void Resources::Shutdown()
{
	// Delete all loaded resources
	for (TMapEntry<string, ResourceData>* res : m_resources)
	{
		delete[] res->Value().data;
	}

	m_resources.Clear();
	m_fileMappings.Clear();
}