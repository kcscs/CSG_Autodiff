#include "SDFGenerator.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "exceptions.h"
#include "ShaderLibManager.h"

#define NOT_ASSIGNED -1

void SDFGenerator::operator()(std::shared_ptr<OperatorNode> opnode)
{
	if (opnode->InputCount() < 1)
		throw shader_gen_exception(shader_gen_exception::REASON::OPERATOR_HAS_NO_INPUTS, opnode);

	// create current transform matrix for this node and save it to the stack
	glm::mat4 transformMatrix = 
		transformStack.top() *
		glm::translate(opnode->translate) *
		glm::rotate(opnode->rotate.z / 180 * glm::pi<float>(), glm::vec3(0, 0, 1)) *
		glm::rotate(opnode->rotate.y / 180 * glm::pi<float>(), glm::vec3(0, 1, 0)) *
		glm::rotate(opnode->rotate.x / 180 * glm::pi<float>(), glm::vec3(1, 0, 0)) *
		glm::scale(glm::vec3(opnode->scale));

	transformStack.push(transformMatrix);

	// traverse children, generate their code
	for (auto nb : *opnode) {
		nb->visit(this);
	}

	transformStack.pop();

	int& reg = TraversalId(opnode); // name of variable where this nodes distance will be saved
	int firstReg = TraversalId(opnode->FirstInput());
	reg = firstReg; // reuse the variable containing the first input
	std::string regName = regNamePrefix + std::to_string(reg); 

	std::vector<std::string> inputRegisters { regName };
	for (int i = 1; i < opnode->InputCount(); ++i) { // collect name of own + input variables for passing it to the operator code generator
		inputRegisters.push_back(regNamePrefix + std::to_string(TraversalId((*opnode)[i])));
	}
	code << regName << " = (";
	try { // pass the generation of this node's code to the actual operator class
		opnode->operatorDescription->GenerateShader(code, inputRegisters);
	}
	catch (partial_shader_gen_exception& e) {
		throw shader_gen_exception(e.reason(), opnode);
	}

	code << ") * " << opnode->scale; // scaling correction

	if (opnode->radius != 0)
		code << " - " << opnode->radius; // offset
	code << ";\n";
		
	// free all variables for later used, except the one which contains the output
	for (int i = 1; i < opnode->InputCount(); ++i) {
		int& id = TraversalId((*opnode)[i]);
		FreeRegister(id);
		id = NOT_ASSIGNED;
	}
	TraversalId(opnode->FirstInput()) = NOT_ASSIGNED;
}

void SDFGenerator::operator()(std::shared_ptr<PrimitiveNode> primnode)
{
	int& reg = TraversalId(primnode);
	reg = AllocateRegister();
	
	std::string regName = regNamePrefix + std::to_string(reg);

	// calculate transform matrix of this node
	glm::mat4 transform = transformStack.top() * glm::translate(primnode->translate) *
		glm::rotate(primnode->rotate.z / 180 * glm::pi<float>(), glm::vec3(0, 0, 1)) *
		glm::rotate(primnode->rotate.y / 180 * glm::pi<float>(), glm::vec3(0, 1, 0)) *
		glm::rotate(primnode->rotate.x / 180 * glm::pi<float>(), glm::vec3(1, 0, 0));

	auto invTransform = glm::inverse(transform); // inverse for moving the sampling point instead of the primitive
 
	if (!createdInvVar) { // define variables for holding the inv transf mtx + the sampling coordinate
		code << "mat4 " << invTransformVarName << ";\n";
		code << "vec3 " << transfSampleCoordName << ";\n";
		createdInvVar = true;
	}

	// fill inv transf mtx
	code << invTransformVarName << "[0] = " << createVec4(invTransform[0]) << ";\n";
	code << invTransformVarName << "[1] = " << createVec4(invTransform[1]) << ";\n";
	code << invTransformVarName << "[2] = " << createVec4(invTransform[2]) << ";\n";
	code << invTransformVarName << "[3] = " << createVec4(invTransform[3]) << ";\n";

	// compute sampling coordinate for sampling the primitive in its basic (non-transformed) form
	code << transfSampleCoordName << " = " << "(" << invTransformVarName << " * vec4(" << sampleCoordName << ",1)).xyz / "<<primnode->scale<<";\n";

	if (reg == registerCount) { // declare variable for storing result if it hasn't been declared yet.
		registerCount++;
		code << "float " << regName << ";\n";
	}

	code << regName << " = (r_";
	primnode->primitive->GenerateShader(code, transfSampleCoordName); // call primitive shader generation 

	if (primnode->radius != 0) 
		code << " - " << primnode->radius; // offset
	
	code << ") * " << primnode->scale; // scaling correction
	code << ";\n";
}

std::string SDFGenerator::GenerateFromRoot(std::shared_ptr<Node> root)
{
	code.clear();
	createdInvVar = false;
	createdTempVec3 = false;
	nextRegister = 0;
	freeRegisters.clear();
	while (!transformStack.empty())
		transformStack.pop();
	transformStack.push(glm::identity<glm::mat4>());

	code << "float sdf(vec3 pos) {\n";
	root->visit(this);
	transformStack.pop();

	code << "return " << regNamePrefix << TraversalId(root) << ";\n}\n";
	TraversalId(root) = NOT_ASSIGNED;
	return ShaderLibManager::GenerateFromTemplate(code.str(), false); //TODO: move templating to primitive/operator code generator
}

int SDFGenerator::AllocateRegister()
{
	if (freeRegisters.size() > 0) {
		int r = freeRegisters.back();
		freeRegisters.pop_back();
		return r;
	}

	return nextRegister++;
}

void SDFGenerator::FreeRegister(int id)
{
	freeRegisters.push_back(id);
}

std::string SDFGenerator::createVec4(glm::vec4 v)
{
	std::stringstream str;
	str << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
	return str.str();
}
