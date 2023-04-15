#include "Persistence.h"
#include <iostream>
#include <fstream>

/// <summary>
/// Saves the current graph of the passed Editor instance to the specified file.
/// </summary>
/// <param name="nodes">- editor instance to save the graph from</param>
/// <param name="filePath">- file to write to</param>
void Persistence::SaveToJson(Editor& nodes, std::string filePath)
{
	std::string nodesAsJsonStr = NodeJsonSerializer::Serialize(nodes.GetRootNodes());
	std::cout << "JSON SAVE ("<<filePath<<")\n" << nodesAsJsonStr << "\n ---- \n";
	try {
		std::ofstream f(filePath);
		f << nodesAsJsonStr;
		f.close();
	}
	catch (std::exception& e) {
		std::cerr <<"ERROR writing file " << filePath << ": " << e.what() << "\n";
	}
}

/// <summary>
/// Reads a json file from disk and attempts to parse and load the nodes from it to the passed Editor instance.
/// Note: it loads nodes additively and might throw exceptions in case the file is not formatted correctly
/// </summary>
/// <param name="nodes">- the Editor instance to add the loaded nodes to</param>
/// <param name="filePath">- the path relative to the executable</param>
void Persistence::LoadFromJson(Editor& nodes, std::string filePath)
{
	std::ifstream f(filePath);
	std::string content(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>{});

	std::cout << "\nLOADED: " << filePath << "\n" << content << "\n";
	f.close();

	nodes.Clear();

	NodeJsonSerializer::DeserializeInto(nodes, content);

	nodes.AutoArrange();
}