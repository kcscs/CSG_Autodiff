#pragma once

#include "NodeVisitor.h"
#include "Editor.h"
#include "utils.h"
#include <vector>
#include <stack>
#include <string>
#include <sstream>

#include <glm/glm.hpp>

/// <summary>
/// Implements a modified depth first traversal of the graph for generating the sdf.
/// Detailed explanation in docs.
/// </summary>
class SDFGenerator : public NodeVisitor
{
public:
	void operator()(std::shared_ptr<OperatorNode> opnode) override;
	void operator()(std::shared_ptr<PrimitiveNode> primnode) override;

	/// <summary>
	/// Convenience function for generating the whole sdf from a given root
	/// </summary>
	/// <param name="root"></param>
	/// <returns> the glsl code of the sdf</returns>
	std::string GenerateFromRoot(std::shared_ptr<Node> root);

private:
	std::stringstream code;
	int nextRegister = 0;
	std::vector<int> freeRegisters;

	std::stack<glm::mat4> transformStack;

	bool createdInvVar = false;
	const std::string invTransformVarName = "inv";

	bool createdTempVec3 = false;
	std::string tempVec3Name = "tmpv3";

	int registerCount = 0;

	const std::string regNamePrefix = "var";
	const std::string sampleCoordName = "pos";
	const std::string transfSampleCoordName = "posTransf";

	int AllocateRegister();
	void FreeRegister(int id);

	std::string createVec4(glm::vec4 v);
};