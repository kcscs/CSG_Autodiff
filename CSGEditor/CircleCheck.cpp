#include "CircleCheck.h"

#include "NodeVisitor.h"

#include <vector>

#define UNVISITED 0
#define CIRCLE_CHECK_FLAG_ENTERED 1
#define CIRCLE_CHECK_FLAG_RETURNED 2

/// <summary>
/// Implements depth first search for detecting circles in the graph
/// </summary>
class CircleCheckVisitor : public NodeVisitor {

	bool found = false;
	std::vector<std::shared_ptr<Node>> affected;

	void Clean() {
		for (auto n : affected)
			Flag(n) = UNVISITED;
		affected.clear();
	}

public:
	virtual void operator()(std::shared_ptr<OperatorNode> opnode) override {
		int& flag = Flag(opnode);

		if (flag == UNVISITED) {
			affected.push_back(opnode);
			flag = CIRCLE_CHECK_FLAG_ENTERED;
			for (auto inptr : *opnode) {
				assert(inptr != nullptr);
				inptr->visit(this);
				if (found)
					break;
			}
			flag = CIRCLE_CHECK_FLAG_RETURNED;
		}
		else if (flag == CIRCLE_CHECK_FLAG_ENTERED) {
			found = true;
		}
	}

	virtual void operator()(std::shared_ptr<PrimitiveNode> primnode) override {
		int& flag = Flag(primnode);

		flag = CIRCLE_CHECK_FLAG_RETURNED;
		affected.push_back(primnode);
	}

	bool Check(std::shared_ptr<Node> root) {
		root->visit(this);
		Clean();
		return found;
	}
};

bool CheckCircleFromNode(std::shared_ptr<Node> root)
{
	CircleCheckVisitor check;
	return check.Check(root);
}

bool CheckCircleFromNodeWithAddedLink(std::shared_ptr<OperatorNode> root, std::shared_ptr<Node> tempNeighbour)
{
	root->AddInputBack(tempNeighbour);
	bool val = CheckCircleFromNode(root);
	root->RemoveLastInput();
	return val;
}
