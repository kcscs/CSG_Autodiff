#include "utils.h"
#include <ImGui/imgui_internal.h>
#include <iostream>

namespace utils {
	bool InputVec3(std::string label, glm::vec3& data, int decimals)
	{
		static float arr[3];
		arr[0] = data.x;
		arr[1] = data.y;
		arr[2] = data.z;
		bool result = ImGui::InputFloat3(label.c_str(), arr, decimals);
		data.x = arr[0];
		data.y = arr[1];
		data.z = arr[2];
		return result;
	}

	ImVec2 AddImVec2(ImVec2 a, ImVec2 b)
	{
			a.x += b.x;
			a.y += b.y;
			return a;
	}

	ImVec2 SubstractImVec2(ImVec2 a, ImVec2 b)
	{
		a.x -= b.x;
		a.y -= b.y;
		return a;
	}

	ImVec2 ScaleImVec2(ImVec2 a, float t)
	{
		a.x *= t;
		a.y *= t;
		return a;
	}

	
	

	

	
}

std::ostream& operator<<(std::ostream& s, const glm::vec3& v) {
	return s << "vec3(" << v.x << ',' << v.y << ',' << v.z << ')';
}