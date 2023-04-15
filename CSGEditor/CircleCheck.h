#pragma once

#include "Node.h"

// Check if there is a circle in the subtree
bool CheckCircleFromNode(std::shared_ptr<Node> root);

/// <summary>
/// Check if adding an edge between root and tempNeighbour forms a circle in the graph
/// </summary>
bool CheckCircleFromNodeWithAddedLink(std::shared_ptr<OperatorNode> root, std::shared_ptr<Node> tempNeighbour);