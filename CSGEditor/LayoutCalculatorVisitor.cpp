#include "LayoutCalculatorVisitor.h"

using NodeHandle = Editor::NodeHandle;

void LayoutCalculatorVisitor::operator()(std::shared_ptr<PrimitiveNode> primNode)
{
	std::shared_ptr<GuiNode> guiNode = primNode->guiNode.lock();
	assert(guiNode != nullptr);

	// primitive node: just read the next available position from the current layer
	int verticalPosition = heightStack[depth].back();

	// save this node's position in the returned map
	positions.emplace(guiNode, ImVec2((float)columnWidth * -depth, (float)verticalPosition));

	// push next node's vertical position on this depth layer
	heightStack[depth].push_back(verticalPosition + height(primNode));
}

void LayoutCalculatorVisitor::operator()(std::shared_ptr<OperatorNode> opNode)
{
	std::shared_ptr<GuiNode> guiNode = opNode->guiNode.lock();
		assert(guiNode != nullptr);

	if (heightStack.size() == depth + 1) // if this is the first node on a new layer
		heightStack.push_back({ heightStack[depth].back() }); // the first child's height should be the height of this node
	
	// the index of the vertical position of the first child
	size_t firstIdx = heightStack[depth + 1].size()-1; // first child will push_back heightStack[depth + 1] with its own height

	int ownVerticalPosition;
	if(opNode->InputCount() == 0)
		ownVerticalPosition = heightStack[depth].back();
	else {
		depth++;
		for (auto inputNode : *opNode) { // this traversal assumes a tree graph (wont work on DAG)
			inputNode->visit(this);
		}
		depth--;

		// the index of the vertical position of the last child
		size_t lastIdx = heightStack[depth + 1].size() - 2;

		// position self in the middle vertically
		if ((lastIdx - firstIdx + 1) % 2 == 0) {
			ownVerticalPosition = (heightStack[depth + 1][(lastIdx + firstIdx) / 2] + heightStack[depth + 1][(lastIdx + firstIdx + 1) / 2]) / 2;
		}
		else {
			ownVerticalPosition = heightStack[depth + 1][(lastIdx + firstIdx) / 2];
		}
	}
	
	// save own position to output map
	positions.emplace(guiNode, ImVec2((float)columnWidth * -depth, (float)ownVerticalPosition));

	heightStack[depth].back() = ownVerticalPosition; // save vertical position
	heightStack[depth].push_back(ownVerticalPosition + height(opNode)); // vertical position of next node on this layer
}

std::unordered_map<NodeHandle, ImVec2> LayoutCalculatorVisitor::CalculateLayout(std::vector<std::shared_ptr<Node>> roots)
{
	positions.clear();
	heightStack.clear();
	depth = 0;

	heightStack.push_back({ 0 });
	for(auto root : roots)
		root->visit(this);

	return positions;
}
