#include "NodeJsonSerializer.h"

ordered_json NodeJsonSerializer::GenerateFromRoot(std::shared_ptr<Node> root)
{
	while (!jsonStack.empty())
		jsonStack.pop();
	jsonStack.emplace();
	depth = 0;

	try {
		root->visit(this);
	}
	catch (json::exception e) {
		std::cerr << e.what() << "\n";
	}
	return jsonStack.top();
}

std::string NodeJsonSerializer::Serialize(std::vector<Editor::NodeHandle> roots)
{
	NodeJsonSerializer serializer;
	ordered_json j;
	for (auto nh : roots) {
		j += serializer.GenerateFromRoot(nh.node->node);
	}

	std::stringstream str;
	str << std::setw(4) << j;

	return str.str();
}

void NodeJsonSerializer::DeserializeInto(Editor& nodes, std::string jsonString)
{
	ordered_json j = ordered_json::parse(jsonString);

	if (j.is_null())
		return;

	if (!j.is_array())
		throw std::exception("Json format error: expected array as root object.");

	for (ordered_json rootObj : j) {
		LoadTree(nodes, rootObj);
	}
}

void NodeJsonSerializer::operator()(std::shared_ptr<PrimitiveNode> primNode)
{
	ordered_json j = {
		{"primitive", primNode->primitive->GetName() },
		{ "translate", primNode->translate },
		{ "rotate", primNode->rotate },
		{ "scale", primNode->scale },
		{ "offset", primNode->radius }
	};
	primNode->primitive->SaveToJson(j);
	if (depth == 0) // this is to avoid an unnecessary json array with the root node as it's single element
		jsonStack.top() = j;
	else
		jsonStack.top() += j;
}

void NodeJsonSerializer::operator()(std::shared_ptr<OperatorNode> opNode)
{
	jsonStack.emplace();
	++depth;
	for (auto& input : *opNode) {
		input->visit(this);
	}
	--depth;
	ordered_json inputList = jsonStack.top();
	jsonStack.pop();

	ordered_json j = {
		{ "operator", opNode->operatorDescription->GetName() },
		{"translate", opNode->translate},
		{ "rotate", opNode->rotate },
		{ "scale", opNode->scale },
		{ "offset", opNode->radius },
	};

	opNode->operatorDescription->SaveToJson(j);

	j["inputs"] = inputList;

	if (depth == 0) // this is to avoid an unnecessary array with the root node as it's single element
		jsonStack.top() = j;
	else
		jsonStack.top() += j;
}


Editor::NodeHandle NodeJsonSerializer::LoadTree(Editor& nodes, ordered_json& j)
{
	std::cout << "LOAD TREE: " << std::setw(2) << j << "\n";
	if (j.contains("operator")) {
		auto opNode = OperatorNode::Create();
		auto typeName = j["operator"].get<std::string>();
		opNode->operatorIdx = OperatorTypes::GetTypeIdx(typeName);		
		opNode->translate = getOrDefault<glm::vec3>(j, "translate", glm::vec3());
		opNode->rotate = getOrDefault<glm::vec3>(j, "rotate", glm::vec3());
		opNode->scale = getOrDefault<float>(j, "scale", 1.0f);
		opNode->radius = getOrDefault<float>(j, "offset", 0.0f);

		opNode->operatorDescription = OperatorTypes::Create(j, typeName);

		auto opNodeHandle = nodes.AddNode(opNode);

		if (j.contains("inputs")) {
			auto inputs = j["inputs"];
			for (ordered_json inputJson : inputs) {
				auto node = LoadTree(nodes, inputJson);
				nodes.ConnectNodes(node, opNodeHandle);
			}
		}

		return opNodeHandle;
	}
	else if (j.contains("primitive")) {

		auto primNode = PrimitiveNode::Create();
		auto typeName = j["primitive"].get<std::string>();
		primNode->primitiveIdx = PrimitiveTypes::GetTypeIdx(typeName);
		primNode->translate = getOrDefault<glm::vec3>(j, "translate", glm::vec3());
		primNode->rotate = getOrDefault<glm::vec3>(j, "rotate", glm::vec3());
		primNode->scale = getOrDefault<float>(j, "scale", 1.0f);
		primNode->radius = getOrDefault<float>(j, "offset", 0.0f);

		primNode->primitive = PrimitiveTypes::Create(j, typeName);

		return nodes.AddNode(primNode);
	}
	else {
		std::cerr << "JSON LOADER: Unknown node type\n";
		return std::shared_ptr<GuiNode>(nullptr);
	}
}



void glm::to_json(ordered_json& j, const glm::vec3& v)
{
	j = { {"x", v.x}, { "y", v.y }, { "z", v.z } };
}

void glm::from_json(const ordered_json& j, glm::vec3& v)
{
	v.x = j["x"].get<float>();
	v.y = j["y"].get<float>();
	v.z = j["z"].get<float>();
}