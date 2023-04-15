#include "Primitive.h"
#include "Persistence.h"

#include <algorithm>

std::vector<std::string> PrimitiveTypes::names;
bool PrimitiveTypes::nameListGenerated;

std::ostream& Sphere::GenerateShader(std::ostream& code, std::string sampleCoordVarName)
{
    return code << "sphere(0.5f, " << sampleCoordVarName << ')';
}

std::unique_ptr<Primitive> Sphere::clone()
{
    auto copy = std::make_unique<Sphere>();
    *copy = *this;
    return copy;
}

bool Box::NodeEditorDraw()
{
    return utils::InputVec3("dimensions", dimensions, 2);
}

std::ostream& Box::GenerateShader(std::ostream& code, std::string sampleCoordVarName)
{
    return code << "cube(" << dimensions * 0.5f << ", " << sampleCoordVarName << ')';
}

void Box::SaveToJson(ordered_json& json)
{
    json["dimensions"] = dimensions;
}

std::unique_ptr<Primitive> Box::clone()
{
    auto copy = std::make_unique<Box>();
    *copy = *this;
    return copy;
}

Box::Box(ordered_json& json)
{
    json.at("dimensions").get_to(dimensions);
}

bool Cylinder::NodeEditorDraw()
{
    return
        ImGui::InputFloat("radius", &radius) ||
        ImGui::InputFloat("height", &height);
}

std::ostream& Cylinder::GenerateShader(std::ostream& code, std::string sampleCoordVarName)
{
    return code << "cylinder(" << radius << ", " << height << ", " << sampleCoordVarName << ')';
}

void Cylinder::SaveToJson(ordered_json& json)
{
    json["height"] = height;
    json["radius"] = radius;
}

std::unique_ptr<Primitive> Cylinder::clone()
{
    auto copy = std::make_unique<Cylinder>();
    *copy = *this;
    return copy;
}

Cylinder::Cylinder(ordered_json& json)
{
    json.at("radius").get_to(radius);
    json.at("height").get_to(height);
}

bool Torus::NodeEditorDraw()
{
    return
        ImGui::InputFloat("major radius", &major_radius) ||
        ImGui::InputFloat("minor radius", &minor_radius);
}

std::ostream& Torus::GenerateShader(std::ostream& code, std::string sampleCoordVarName)
{
    return code << "torus(" << major_radius << ", " << minor_radius << ", " << sampleCoordVarName << ')';
}

void Torus::SaveToJson(ordered_json& json)
{
    json["major_radius"] = major_radius;
    json["minor_radius"] = minor_radius;
}

std::unique_ptr<Primitive> Torus::clone()
{
    auto copy = std::make_unique<Torus>();
    *copy = *this;
    return copy;
}

Torus::Torus(ordered_json& json)
{
    json.at("major_radius").get_to(major_radius);
    json.at("minor_radius").get_to(minor_radius);
}

bool Ellipsoid::NodeEditorDraw()
{
    return utils::InputVec3("radii", radii, 2);
}

std::ostream& Ellipsoid::GenerateShader(std::ostream& code, std::string sampleCoordVarName)
{
    return code << "ellipsoid(" << radii << ", " << sampleCoordVarName << ')';
}

void Ellipsoid::SaveToJson(ordered_json& json)
{
    json["radii"] = radii;
}

std::unique_ptr<Primitive> Ellipsoid::clone()
{
    auto copy = std::make_unique<Ellipsoid>();
    *copy = *this;
    return copy;
}

Ellipsoid::Ellipsoid(ordered_json& json)
{
    json.at("radii").get_to(radii);
}

bool Plane::NodeEditorDraw()
{
    bool changed = false;
    if (ImGui::InputFloat3("n", &n[0], 3, ImGuiInputTextFlags_EnterReturnsTrue)) {
        changed = true;
        n = glm::normalize(n);
    }
    return changed || ImGui::InputFloat("h", &h);
}

std::ostream& Plane::GenerateShader(std::ostream& code, std::string sampleCoordVarName)
{
    return code << "plane(" << n << ", " << h << ", " << sampleCoordVarName << ')';
}

Plane::Plane(ordered_json& json)
{
    json.at("n").get_to(n);
    json.at("h").get_to(h);
}

void Plane::SaveToJson(ordered_json& json)
{
    json["n"] = n;
    json["h"] = h;
}

std::unique_ptr<Primitive> Plane::clone()
{
    auto copy = std::make_unique<Plane>();
    *copy = *this;
    return copy;
}
