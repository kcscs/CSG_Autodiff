#pragma once

#include "Node.h"

class NodeVisitor
{
public:
	virtual void operator()(std::shared_ptr<OperatorNode> opnode) = 0;
	virtual void operator()(std::shared_ptr<PrimitiveNode> primnode) = 0;

protected:
	int& Flag(std::shared_ptr<Node> node) { return node->flag; }
	int& TraversalId(std::shared_ptr<Node> node) { return node->traversalId; }
};

