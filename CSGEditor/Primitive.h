#pragma once
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <json.hpp>
#include "utils.h"

using namespace nlohmann;

#define PRIMITIVES Sphere, Box, Cylinder, Torus, Ellipsoid, Plane

class Primitive
{
public:
	virtual bool NodeEditorDraw() { return false; };
	virtual bool NodeEditorPopup() { return false; };

	virtual void SaveToJson(ordered_json& json) {};
	virtual std::unique_ptr<Primitive> clone() = 0;
	virtual std::string GetName() = 0;

	virtual std::ostream& GenerateShader(std::ostream& code, std::string sampleCoordVarName) = 0;
};

class Sphere : public Primitive {
public:
	virtual std::ostream& GenerateShader(std::ostream& code, std::string sampleCoordVarName) override;
	virtual std::string GetName() override { return "sphere"; }

	virtual std::unique_ptr<Primitive> clone() override;

	Sphere() {}
	Sphere(ordered_json& json) {}
};

class Box : public Primitive {
public:
	virtual bool NodeEditorDraw() override;

	virtual std::ostream& GenerateShader(std::ostream&  code, std::string sampleCoordVarName) override;
	virtual std::string GetName() override { return "box"; }
	virtual void SaveToJson(ordered_json& json) override;
	virtual std::unique_ptr<Primitive> clone() override;

	Box() : dimensions(1.0f,1.0f,1.0f) {}
	Box(ordered_json& json);
private:
	glm::vec3 dimensions;
};

class Cylinder : public Primitive {
public:
	virtual bool NodeEditorDraw() override;

	virtual std::ostream& GenerateShader(std::ostream& code, std::string sampleCoordVarName) override;
	virtual std::string GetName() override { return "cylinder"; }
	virtual void SaveToJson(ordered_json& json) override;
	virtual std::unique_ptr<Primitive> clone() override;

	Cylinder() : height(1.0f), radius(0.5f) {}
	Cylinder(ordered_json& json);
private:
	float radius;
	float height;
};

class Torus : public Primitive {
public:
	virtual bool NodeEditorDraw() override;
	virtual std::ostream& GenerateShader(std::ostream& code, std::string sampleCoordVarName) override;
	virtual std::string GetName() override { return "torus"; }
	virtual void SaveToJson(ordered_json& json) override;
	virtual std::unique_ptr<Primitive> clone() override;

	Torus() : major_radius(0.4f), minor_radius(0.1f)  {}
	Torus(ordered_json& json);
private:
	float major_radius;
	float minor_radius;
};

class Ellipsoid : public Primitive {
public:
	virtual bool NodeEditorDraw() override;
	virtual std::ostream& GenerateShader(std::ostream& code, std::string sampleCoordVarName) override;
	virtual std::string GetName() override { return "ellipsoid"; }
	virtual void SaveToJson(ordered_json& json) override;
	virtual std::unique_ptr<Primitive> clone() override;

	Ellipsoid() : radii(0.5f, 0.3f, 0.2f) {}
	Ellipsoid(ordered_json& json);
private:
	glm::vec3 radii;
};

class Plane : public Primitive {
public:
	virtual bool NodeEditorDraw() override;
	virtual std::ostream& GenerateShader(std::ostream& code, std::string sampleCoordVarName) override;
	virtual std::string GetName() override { return "plane"; }
	virtual void SaveToJson(ordered_json& json) override;
	virtual std::unique_ptr<Primitive> clone() override;

	Plane() : n(0.0f,1.0f,0.0f), h(0.0f) {}
	Plane(ordered_json& json);
private:
	glm::vec3 n;
	float h;
};

using PrimitiveTypes = utils::TypeList<Primitive, PRIMITIVES>;