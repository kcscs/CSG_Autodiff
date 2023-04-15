#pragma once
#include <string>
#include <fstream>
#include <regex>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>

class ShaderLibManager
{
public:
	static const std::string templatePrimitiveLibFile;
	static const std::string realPrimitiveLibFile;
	static const std::string dualPrimitiveLibFile;
	static const std::string dualNumberTypeName;
	static const std::string dualNumberTypeDataMemberName;

	static std::vector<size_t> CalculateTriangularNumbers(size_t n);
	static std::vector<size_t> CalculateTetrahedralNumbers(size_t n);
	static std::vector<std::vector<size_t>> CalculateNChooseK(size_t n);

	struct NameTableEntry {
		std::string templateName, realName, dualName;
	};

	/// <summary>
	/// Reads the primitive shader library from templatePrimitiveLibFile, replaces all uses of dual numbers to float and vec3, then writes the resulting shader to realPrimitiveLibFile.
	/// </summary>
	static void GeneratePrimitiveLibs();

	/// <summary>
	/// Generates code from templates with string substitutions.
	/// </summary>
	/// <param name="str"> - the template source</param>
	/// <param name="dual"> - whether to generate a dual or real variant</param>
	/// <returns></returns>
	static std::string GenerateFromTemplate(std::string str, bool dual);

	/// <summary>
	/// Generates a glsl function that takes a dual number and returns a dual number populated by the (real) function's output and it's derivatives.
	/// </summary>
	/// <param name='name'> - The name of the generated glsl function.</param>
	/// <param name='func'> - Contains the function and it's derivatives as strings. The function's parameter must be named x. eg.: {"x*x", "2*x"}</param>
	static std::string GenerateChainRuleFunc(std::string name, std::vector<std::string> func, size_t derivative_order);

	/// <summary>
	/// Generate the files containing the constants and settings for a given derivative order.
	/// </summary>
	/// <param name="derivativeOrder"></param>
	/// <returns></returns>
	static std::string GenerateConstants(int derivativeOrder);

private:
	static bool NextPartition(std::vector<int>& partition, std::vector<int>& max);

	template<typename T>
	static std::string CreateConstGlslArray(const std::string& name, const std::string& glslType, const std::vector<T>& vals) {
		std::stringstream code;
		code << "const " << glslType << " " << name << "[" << vals.size() << "] = {\n";
		code << "    " << vals.front();
		for (int i = 1; i < vals.size(); ++i) {
			code << "," << vals[i];
		}
		code << "\n};\n";

		return code.str();
	}

	template<typename T>
	static std::string CreateConstGlslArray2D(const std::string& name, const std::string& glslType, const std::vector<std::vector<T>>& vals) {
		std::stringstream code;
		code << "const " << glslType << " " << name << "[" << vals.size() << "][" << vals.front().size() << "] = {\n";
		for (auto& row : vals) {
			code << "    {" << row.front();
			for (int i = 1; i < row.size(); ++i) {
				code << "," << row[i];
			}
			code << "},\n";
		}
		code << "};\n";

		return code.str();
	}
};

