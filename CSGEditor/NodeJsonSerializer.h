#pragma once
#include "NodeVisitor.h"
#include "Editor.h"
#include <stack>
#include <sstream>

/// <summary>
/// Uses a modified depth first traversal of the graph for serializing its nodes.
/// Also responsible for deserializing and adding the loaded graph to an Editor instance
/// Details in docs.
/// </summary>
class NodeJsonSerializer : public NodeVisitor {
public:
	void operator()(std::shared_ptr<PrimitiveNode> primNode);
	void operator()(std::shared_ptr<OperatorNode> opNode);
	ordered_json GenerateFromRoot(std::shared_ptr<Node> root);

	static std::string Serialize(std::vector<Editor::NodeHandle> roots);
	static void DeserializeInto(Editor& nodes, std::string jsonString);
private:
	static Editor::NodeHandle LoadTree(Editor& nodes, ordered_json& root);
	std::stack<ordered_json> jsonStack;
	int depth;

	template<typename t>
	static t getOrDefault(ordered_json& j, std::string key, t default_value);
};

namespace glm {
	void to_json(ordered_json& j, const glm::vec3& v);
	void from_json(const ordered_json& j, glm::vec3& v);
}


template<typename t>
inline t NodeJsonSerializer::getOrDefault(ordered_json& j, std::string key, t default_value)
{
	if (j.contains(key))
		return j[key].get<t>();
	return default_value;
}