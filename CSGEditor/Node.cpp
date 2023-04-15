#include "Node.h"
#include "NodeVisitor.h"
#include "GuiPrimitiveNode.h"

void OperatorNode::visit(NodeVisitor* visitor)
{
	auto opnode = std::static_pointer_cast<OperatorNode>(shared_from_this());
	visitor->operator()(opnode);
}

void OperatorNode::UpdateInputs(std::vector<std::shared_ptr<Node>> newInputList)
{
	inputs = newInputList;
}

std::shared_ptr<Node> OperatorNode::clone()
{
	auto clone = OperatorNode::Create();
	clone->translate = translate;
	clone->rotate = rotate;
	clone->scale = scale;
	clone->radius = radius;
	clone->guiNode = std::shared_ptr<GuiPrimitiveNode>(nullptr); // should be assigned after the copy is done, by the class doing the copy
	clone->operatorIdx = operatorIdx;
	clone->operatorDescription = operatorDescription->clone();
	return clone;
}

void OperatorNode::RemoveInput(std::shared_ptr<Node> node)
{
	auto it = std::find_if(inputs.begin(), inputs.end(), [&](std::shared_ptr<Node> inp) {return inp == node; });
	inputs.erase(it);
}

void PrimitiveNode::visit(NodeVisitor* visitor)
{
	auto primnode = std::static_pointer_cast<PrimitiveNode>(shared_from_this());
	visitor->operator()(primnode);
}

std::shared_ptr<Node> PrimitiveNode::clone()
{
	auto clone = PrimitiveNode::Create();
	clone->translate = translate;
	clone->rotate = rotate;
	clone->scale = scale;
	clone->radius = radius;
	clone->guiNode = std::shared_ptr<GuiPrimitiveNode>(nullptr); // should be assigned after the copy is done, by the class doing the copy
	clone->primitiveIdx = primitiveIdx;
	clone->primitive = primitive->clone();
	return clone;
}