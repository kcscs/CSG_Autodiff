#pragma once
#include "NodeVisitor.h"
#include "Editor.h"
#include <unordered_map>
#include <vector>

/// <summary>
/// Responsible for automatically positioning nodes after a graph is loaded.
/// </summary>
class LayoutCalculatorVisitor : public NodeVisitor
{
	using NodeHandle = Editor::NodeHandle;
public:
	virtual void operator()(std::shared_ptr<PrimitiveNode> primNode) override;
	virtual void operator()(std::shared_ptr<OperatorNode> opNode) override;

	/// <summary>
	/// Calculates the layout of the whole forest passed as argument.
	/// </summary>
	/// <param name="roots">Roots of tree graphs.</param>
	/// <returns>A map containing each node's suggested position</returns>
	std::unordered_map<NodeHandle, ImVec2> CalculateLayout(std::vector<std::shared_ptr<Node>> roots);
private:
	std::unordered_map<NodeHandle, ImVec2> positions;

	/// <summary>
	/// inner vector contains vertical positions, outer vector represents different depth values
	/// the last element in the inner vector is the vertical position the next node should be put to on that particular depth layer
	/// </summary>
	std::vector<std::vector<int>> heightStack;
	int depth;

	const int columnWidth = 300;
	const int height(std::shared_ptr<OperatorNode> n) const { return 200; }
	const int height(std::shared_ptr<PrimitiveNode> n) const { return 200; }
};

