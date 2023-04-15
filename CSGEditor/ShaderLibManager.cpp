#include "ShaderLibManager.h"

const std::string ShaderLibManager::templatePrimitiveLibFile = "Shaders/primitives.frag";
const std::string ShaderLibManager::realPrimitiveLibFile = "Shaders/tmp/primitives_real.frag";
const std::string ShaderLibManager::dualPrimitiveLibFile = "Shaders/tmp/primitives_dual.frag";
const std::string ShaderLibManager::dualNumberTypeName = "dnum";
const std::string ShaderLibManager::dualNumberTypeDataMemberName = "d";


std::vector<size_t> ShaderLibManager::CalculateTriangularNumbers(size_t n)
{
	std::vector<size_t> v;
	v.resize(n+1);
	v[0] = 0;
	v[1] = 1;
	size_t step = 2;
	for (size_t i = 2; i <= n; ++i) {
		v[i] = v[i - 1] + step;
		++step;
	}

	return v;
}

std::vector<size_t> ShaderLibManager::CalculateTetrahedralNumbers(size_t n)
{
	std::vector<size_t> v;
	v.resize(n+1);

	v[0] = 0;
	v[1] = 1;
	size_t step = 3;
	size_t substep = 3;
	for (size_t i = 2; i <= n; ++i) {
		v[i] = v[i - 1] + step;
		step += substep;
		++substep;
	}

	return v;
}

std::vector<std::vector<size_t>> ShaderLibManager::CalculateNChooseK(size_t n)
{
	std::vector<std::vector<size_t>> result;
	result.resize(n);
	for (auto& v : result) {
		v.resize(n, 0);
		v[0] = 1;
	}

	for (int i = 1; i < n; ++i) {
		for (int j = 1; j < n; ++j) {
			result[i][j] = result[i - 1][j - 1] + result[i - 1][j];
		}
	}
	return result;
}

void ShaderLibManager::GeneratePrimitiveLibs() {
	std::ifstream templateFile(templatePrimitiveLibFile);
	std::string template_str(std::istreambuf_iterator<char>(templateFile), std::istreambuf_iterator<char>{});
	templateFile.close();

	// \s matches any whitespace
	//	$n:	n-th backreference (i.e., a copy of the n-th matched group specified with parentheses in the regex pattern).
	//		n must be an integer value designating a valid backreference, greater than 0, and of two digits at most.
	std::string real_str = GenerateFromTemplate(template_str, false);

	std::ofstream realFile(realPrimitiveLibFile);
	realFile << real_str;
	realFile.close();

	std::string dual_str = GenerateFromTemplate(template_str, true);

	std::ofstream dualFile(dualPrimitiveLibFile);
	dualFile << dual_str;
	dualFile.close();
}

std::string ShaderLibManager::GenerateFromTemplate(std::string str, bool dual) {
	const static std::vector<NameTableEntry> functionNameTable = {
		{"_dnum_", "float", "dnum"},
		{"_dnum2_", "vec2", "dnum2"},
		{"_dnum3_", "vec3", "dnum3"},

		{"_zero_", "r_zero", "zero"},
		{"_conj_", "r_conj", "conj"},
		{"_realValue_", "r_realValue", "realValue"},
		{"_isReal_", "r_isReal", "isReal"},
		{"_neg_", "r_neg", "neg"},
		{"_add_", "r_add", "add"},
		{"_add3_", "r_add3", "add3"},
		{"_sub_", "r_sub", "sub"},
		{"_sub3_", "r_sub3", "sub3"},
		{"_mul_", "r_mul", "mul"},
		{"_mul3_", "r_mul3", "mul3"},
		{"_div_", "r_div", "div"},
		{"_div3_", "r_div3", "div3"},
		{"_constant_", "r_constant", "constant"},
		{"_constant3_", "r_constant3", "constant3"},
		{"_variable_", "r_variable", "variable"},
		{"_variable3_", "r_variable3", "variable3"},

		{"_dabs_", "r_dabs", "dabs"},
		{"_dabs3_", "r_dabs3", "dabs3"},
		{"_dmin_", "r_dmin", "dmin"},
		{"_dmin3_", "r_dmin3", "dmin3"},
		{"_dmax_", "r_dmax", "dmax"},
		{"_dmax3_", "r_dmax3", "dmax3"},
		{"_dclamp_", "r_dclamp", "dclamp"},
		{"_dmix_", "r_dmix", "dmix"},
		{"_dsqrt_", "r_dsqrt", "dsqrt"},
		{"_dlength_", "r_dlength", "dlength"},

		{"_dsin_", "r_dsin", "dsin"},
		{"_dcos_", "r_dcos", "dcos"},
		{"_ddot_", "r_ddot", "ddot"}
	};

	for (auto entry : functionNameTable) {
		str = std::regex_replace(str, std::regex(entry.templateName), dual ? entry.dualName : entry.realName);
	}

	str = std::regex_replace(str, std::regex("_TEMPLATE_"), dual ? "d_" : "r_");
	return str;
}

std::string ShaderLibManager::GenerateChainRuleFunc(std::string name, std::vector<std::string> func, size_t derivative_order) {
	std::stringstream code;
	std::string indent = "    ";
	code << dualNumberTypeName << " " << name << "(" << dualNumberTypeName << " d) {\n";
	code << indent << "float tmp;\n";
	code << indent << dualNumberTypeName << " result = zero();\n";
	code << indent << "result." << dualNumberTypeDataMemberName << "[0] = "
		<< std::regex_replace(func[0], std::regex("x"), "d." + dualNumberTypeDataMemberName + "[0]") << ";\n";

	for (int x = 0; x <= derivative_order; ++x) {
		size_t yend = derivative_order - x;
		for (size_t y = 0; y <= yend; ++y) {
			size_t zend = yend - y;
			for (size_t z = 0; z <= zend; ++z) {
				if (x + y + z == 0)
					continue;

				// generate all partitions (https://stackoverflow.com/questions/30893292/generate-all-partitions-of-a-set)
				// number of derivations: N = x+y+z
				// set to partition: 1,2,3,...,N
				std::vector<int> partition, max;
				size_t N = x + y + z;
				partition.resize(N, 1);
				max.resize(N, 1);
				max[0] = 0;

				//enumerate partitions
				// each group inside a partition is the input dual number (X) differentiated by the derivations corresponding to the indices inside the group
				// multiply these dual numbers from each group together
				// multiply the previous product by f^(s) (x) where s = the number of groups inside this given partition
				// sum all products generated from all partitions
				// save result into the part of the resulting dual number which corresponds to the (x,y,z) partial derivative
				do
				{
					int group_count = std::max(partition.back(), max.back());
					std::vector<glm::vec3> partial_derivatives;
					partial_derivatives.resize(group_count, glm::vec3(0, 0, 0));

					for (size_t i = 0; i < x; ++i) {
						// 1 <= partition[i] <= N
						partial_derivatives[partition[i] - 1].x++;
					}
					for (size_t i = x; i < x + y; ++i) {
						// 1 <= partition[i] <= N
						partial_derivatives[partition[i] - 1].y++;
					}
					for (size_t i = x + y; i < N; ++i) { // N = x+y+z
						// 1 <= partition[i] <= N
						partial_derivatives[partition[i] - 1].z++;
					}

					code << indent << "tmp = " << std::regex_replace(func[group_count], std::regex("x"), "d." + dualNumberTypeDataMemberName + "[0]") << ";\n";
					for (int i = 0; i < partial_derivatives.size(); ++i) {
						code << indent << "tmp *= d." << dualNumberTypeDataMemberName << "[IDX("
							<< partial_derivatives[i].x << ", "
							<< partial_derivatives[i].y << ", "
							<< partial_derivatives[i].z << ")];\n";
					}
					code << indent << "result." << dualNumberTypeDataMemberName << "[IDX("
						<< x << ", " << y << ", " << z << ")] += tmp;\n";
				} while (NextPartition(partition, max));

			}
		}
	}
	code << indent << "return result;\n";
	code << "}\n";
	return code.str();
}

std::string ShaderLibManager::GenerateConstants(int derivativeOrder) {
	std::stringstream code;

	code << "#version 460\n";
	//code << "#pragma optionNV(unroll none)\n";
	//#pragma optionNV(inline 0) // faster linking at a cost of performance
	code << "#define DERIVATIVE_ORDER " << derivativeOrder << "\n";

	if (derivativeOrder > 0)
		code << "#define DERIVATIVES_ENABLED\n";

	std::vector<size_t> tetrahedralNumbers = CalculateTetrahedralNumbers(derivativeOrder + 1); // +1 to avoid length of 0
	code << CreateConstGlslArray("tetra", "int", tetrahedralNumbers);
	std::vector<size_t> triangularNumbers = CalculateTriangularNumbers(derivativeOrder + 1); // +1 needed
	code << CreateConstGlslArray("tri", "int", triangularNumbers);
	std::vector<std::vector<size_t>> pascalTriangle = CalculateNChooseK(derivativeOrder + 1); // +1 needed
	code << CreateConstGlslArray2D("choose", "int", pascalTriangle);
	code << "#define SIZE " << tetrahedralNumbers[derivativeOrder + 1] << "\n";

	//code << "#define SIZE " << 4 << "\n";
	return code.str();
}

bool ShaderLibManager::NextPartition(std::vector<int>& partition, std::vector<int>& max) {
	for (size_t i = partition.size() - 1; i > 0; --i) { // 0th element must be 1
		if (partition[i] <= max[i]) {
			++partition[i];
			for (size_t j = i + 1; j < partition.size(); ++j) {
				partition[j] = 1;
				max[j] = std::max(max[j - 1], partition[j - 1]);
			}
			return true;
		}
	}

	return false;
}