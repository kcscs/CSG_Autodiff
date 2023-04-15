#pragma once
#include <glm/vec3.hpp>
#include <memory>
#include <vector>
#include <variant>

#include <json.hpp>

#include "Primitive.h"
#include "Operator.h"

class NodeVisitor;
struct GuiNode;

// IMPORTANT: don't forget to update the clone function for added fields (PrimitiveNode, OperatorNode)
struct Node : public std::enable_shared_from_this<Node>
{
	friend class NodeVisitor;

	/// <summary>
	/// Translation vector for this node's output
	/// </summary>
	glm::vec3 translate;
	/// <summary>
	/// Rotation in euler axes (roatation around x,y and z axes)
	/// </summary>
	glm::vec3 rotate;
	float scale;
	/// <summary>
	/// Also known as offset
	/// </summary>
	float radius;

	std::weak_ptr<GuiNode> guiNode;

	virtual void visit(NodeVisitor* visitor) = 0;
	virtual std::shared_ptr<Node> clone() = 0;

protected:
	Node() : translate(0), rotate(0), scale(1), radius(0) {}

private:
	int flag = 0;
	int traversalId = -1;
};

// IMPORTANT: don't forget to update the clone function for added fields
struct OperatorNode : public Node
{
	size_t operatorIdx = 0;
	std::unique_ptr<Operator> operatorDescription;

	virtual void visit(NodeVisitor* visitor) override;
	virtual std::shared_ptr<Node> clone() override;
	void UpdateInputs(std::vector<std::shared_ptr<Node>> newInputList);

	static std::shared_ptr<OperatorNode> Create() { return std::shared_ptr<OperatorNode>(new OperatorNode()); }

private:
	std::vector<std::shared_ptr<Node>> inputs;
	OperatorNode() : Node(), operatorIdx(0), operatorDescription(new Union()) {}

public:
	decltype(inputs)::iterator begin() { return inputs.begin(); }
	decltype(inputs)::iterator end() { return inputs.end(); }
	std::shared_ptr<Node> FirstInput() { return inputs.front(); }
	size_t InputCount() const { return inputs.size(); }
	std::shared_ptr<Node>& operator[](int idx) { return inputs[idx]; }
	void RemoveInput(std::shared_ptr<Node> node);
	void AddInputBack(std::shared_ptr<Node> node) { inputs.push_back(node); }
	void RemoveLastInput() { inputs.pop_back(); }
	void ClearInputs() { inputs.clear(); }
};

// IMPORTANT: don't forget to update the clone function for added fields
struct PrimitiveNode : public Node
{
	size_t primitiveIdx = 0;
	std::unique_ptr<Primitive> primitive;
	
	virtual void visit(NodeVisitor* visitor) override;
	virtual std::shared_ptr<Node> clone() override;

	static std::shared_ptr<PrimitiveNode> Create() { return std::shared_ptr<PrimitiveNode>(new PrimitiveNode()); }

private:
	PrimitiveNode() : primitiveIdx(0), primitive(new Sphere()) {}
};