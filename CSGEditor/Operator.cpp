#include "Operator.h"
#include "exceptions.h"

std::vector<std::string> OperatorTypes::names;
bool OperatorTypes::nameListGenerated;

std::ostream& Union::GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) {
	for (int i = 0; i < inputRegisterNames.size() - 1; ++i) {
		code << "_dmin_(";
	}

	code << inputRegisterNames[0];

	for (int i = 1; i < inputRegisterNames.size(); ++i) {
		code << ", " << inputRegisterNames[i] << ')';
	}

	return code;
}

std::ostream& Intersection::GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) {
	for (int i = 0; i < inputRegisterNames.size() - 1; ++i) {
		code << "_dmax_(";
	}

	code << inputRegisterNames[0];

	for (int i = 1; i < inputRegisterNames.size(); ++i) {
		code << ", " << inputRegisterNames[i] << ')';
	}

	return code;
}

std::ostream& Substraction::GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) {
	for (int i = 0; i < inputRegisterNames.size() - 1; ++i) {
		code << "_dmax_(";
	}

	code << inputRegisterNames[0];

	for (int i = 1; i < inputRegisterNames.size(); ++i) {
		code << ", _neg_(" << inputRegisterNames[i] << "))";
	}

	return code;
}

std::ostream& SmoothUnion::GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames)
{
	SmoothOperator::GenerateShader(code, inputRegisterNames);
	return code << "_TEMPLATE_smooth_union(" << inputRegisterNames[0] << ", " << inputRegisterNames[1] << ", " << k << ")";
}

std::ostream& SmoothIntersection::GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames)
{
	SmoothOperator::GenerateShader(code, inputRegisterNames);
	return code << "_TEMPLATE_smooth_intersection(" << inputRegisterNames[0] << ", " << inputRegisterNames[1] << ", " << k << ")";
}

std::ostream& SmoothOperator::GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames)
{
	if (inputRegisterNames.size() != 2) {
		throw partial_shader_gen_exception(shader_gen_exception::REASON::SMOOTH_OPERATOR_NEEDS_EXACTLY_TWO_INPUTS); // ugly: will be caught by the shadergenerator and the source will be filled there before rethrow
	}
	return code;
}

std::ostream& SmoothSubstraction::GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames)
{
	SmoothOperator::GenerateShader(code, inputRegisterNames);
	return code << "_TEMPLATE_smooth_substraction(" << inputRegisterNames[0] << ", " << inputRegisterNames[1] << ", " << k << ")";
}
