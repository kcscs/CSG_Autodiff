#pragma once

#include "Editor.h"
#include "NodeVisitor.h"
#include "Node.h"
#include "Editor.h"
#include "NodeJsonSerializer.h"

#include <stack>

using namespace nlohmann;

class Persistence
{
public:
	using NodeHandle = Editor::NodeHandle;

	/// <summary>
	/// Saves the graph in the passed editor to the file at the given path.
	/// </summary>
	/// <param name="nodes">- the graph editor</param>
	/// <param name="fileName">- the file to save the graph to</param>
	static void SaveToJson(Editor& nodes, std::string fileName);

	/// <summary>
	/// Loads the graph into the passed editor from the file at the given path.
	/// </summary>
	/// <param name="nodes">- the graph editor</param>
	/// <param name="fileName">- the file to load the graph from</param>
	static void LoadFromJson(Editor& nodes, std::string fileName);
};
