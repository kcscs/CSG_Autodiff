#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "utils.h"

using namespace nlohmann;

#define OPERATORS Union, Intersection, Substraction, SmoothUnion, SmoothIntersection, SmoothSubstraction

class Operator
{
public:
	virtual bool NodeEditorDraw() { return false; };
	virtual bool NodeEditorPopup() { return false; };

	virtual void SaveToJson(ordered_json& json) {};
	virtual std::unique_ptr<Operator> clone() = 0;
	virtual std::string GetName() = 0;

	virtual std::ostream& GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) = 0;
};

class Union : public Operator {
public:
	virtual std::string GetName() override { return "union"; }
	virtual std::ostream& GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) override;
	virtual std::unique_ptr<Operator> clone() override { return std::make_unique<Union>(*this); };

	Union() {}
	Union(ordered_json& json) {}
};

class Intersection : public Operator {
public:
	virtual std::string GetName() override { return "intersect"; }
	virtual std::ostream& GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) override;
	virtual std::unique_ptr<Operator> clone() override { return std::make_unique<Intersection>(*this); };

	Intersection() {}
	Intersection(ordered_json& json) {}
};

class Substraction : public Operator {
public:
	virtual std::string GetName() override { return "substract"; }
	virtual std::ostream& GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) override;
	virtual std::unique_ptr<Operator> clone() override { return std::make_unique<Substraction>(*this); };

	Substraction() {}
	Substraction(ordered_json& json) {}
};

class SmoothOperator : public Operator {
public:
	virtual bool NodeEditorDraw() { return ImGui::InputFloat("k", &k); };
	virtual void SaveToJson(ordered_json& json) override { json["k"] = k; };
	virtual std::ostream& GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) override;

	SmoothOperator() {}
	SmoothOperator(ordered_json& json) { json.at("k").get_to<float>(k); }
protected:
	float k = 0.3f;
};

class SmoothUnion : public SmoothOperator {
public:
	virtual std::string GetName() override { return "smooth union"; }
	virtual std::ostream& GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) override;
	virtual std::unique_ptr<Operator> clone() override { return std::make_unique<SmoothUnion>(*this); };

	SmoothUnion() {}
	SmoothUnion(ordered_json& json) : SmoothOperator(json) {}
};

class SmoothIntersection : public SmoothOperator {
public:
	virtual std::string GetName() override { return "smooth intersect"; }
	virtual std::ostream& GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) override;
	virtual std::unique_ptr<Operator> clone() override { return std::make_unique<SmoothIntersection>(*this); };

	SmoothIntersection() {}
	SmoothIntersection(ordered_json& json) : SmoothOperator(json) {}
};

class SmoothSubstraction : public SmoothOperator {
public:
	virtual std::string GetName() override { return "smooth substract"; }
	virtual std::ostream& GenerateShader(std::ostream& code, std::vector<std::string> inputRegisterNames) override;
	virtual std::unique_ptr<Operator> clone() override { return std::make_unique<SmoothSubstraction>(*this); };

	SmoothSubstraction() {}
	SmoothSubstraction(ordered_json& json) : SmoothOperator(json) {}
};

using OperatorTypes = utils::TypeList<Operator, OPERATORS>;