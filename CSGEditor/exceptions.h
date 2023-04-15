#pragma once
#include <exception>
#include <memory>

#include "forward_declarations.h"

#include "Editor.h"
#include "Operator.h"

class shader_gen_exception : public std::exception {
public:
	enum class REASON { OPERATOR_HAS_NO_INPUTS, SMOOTH_OPERATOR_NEEDS_EXACTLY_TWO_INPUTS };
	const char* what() const noexcept {
		return "Shader generation failed";
	}

	shader_gen_exception(REASON reason, std::shared_ptr<OperatorNode> source) : _reason(reason), _source(source->guiNode.lock()) {}

	REASON reason() const { return _reason; }
	Editor::NodeHandle source() const { return _source; }

private:
	REASON _reason;
	std::shared_ptr<GuiNode> _source;
};

class partial_shader_gen_exception : public std::exception {
public:
	const char* what() const noexcept {
		return "Shader generation failed (p)";
	}

	partial_shader_gen_exception(shader_gen_exception::REASON reason) : _reason(reason) {}
	shader_gen_exception::REASON reason() const { return _reason; }
private:
	shader_gen_exception::REASON _reason;
};
