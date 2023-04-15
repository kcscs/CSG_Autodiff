#pragma once
#include <glm/vec3.hpp>
#include <ImGui/imgui.h>
#include <ImGui-addons/imgui_node_editor/imgui_node_editor.h>
#include <string>
#include <vector>
#include <json.hpp>

using namespace ax;
using namespace nlohmann;

namespace utils {
	bool InputVec3(std::string label, glm::vec3& data, int decimals);

	ImVec2 AddImVec2(ImVec2 a, ImVec2 b);
	ImVec2 SubstractImVec2(ImVec2 a, ImVec2 b);
	ImVec2 ScaleImVec2(ImVec2 a, float t);

	template<typename EnumType>
	class ComboBoxFix {
	public:
		ComboBoxFix(std::string label);
		void Init(std::vector<std::string>* options, EnumType* selectedIndex);
		void Draw();
		bool DrawPopup();
	private: 
		std::string label;
		std::vector<std::string>* options = nullptr;
		EnumType* selectedIndex = nullptr;

		std::string popupName;
		ImVec2 popupPos;
		bool initialized;
	};

	template<typename EnumType>
	ComboBoxFix<EnumType>::ComboBoxFix(std::string label)
		: label(label), popupName("ComboBoxFix" + label), initialized(false), options(nullptr), selectedIndex(nullptr)
	{
	}

	template<typename EnumType>
	void ComboBoxFix<EnumType>::Init(std::vector<std::string>* options, EnumType* selectedIndex)
	{
		this->options = options;
		this->selectedIndex = selectedIndex;
		this->initialized = true;
	}

	template<typename EnumType>
	void ComboBoxFix<EnumType>::Draw()
	{
		assert(initialized);
		if (ImGui::Button((*options)[(int) * selectedIndex].c_str())) {
			ImGui::OpenPopup(popupName.c_str());
			popupPos = NodeEditor::CanvasToScreen(ImGui::GetCursorScreenPos());
		}
		else {
			ImGui::SameLine();
			ImGui::Text(label.c_str());
		}
	}

	template<typename EnumType>
	bool ComboBoxFix<EnumType>::DrawPopup()
	{
		assert(initialized);

		EnumType currentIndex = *selectedIndex;

		bool changed = false;
		NodeEditor::Suspend();
		if (ImGui::BeginPopup(popupName.c_str())) {
			ImGui::SetWindowPos(popupPos);
			for (int i = 0; i < options->size(); ++i) {
				if (ImGui::Button((*options)[i].c_str())) {
					*selectedIndex = (EnumType)i;
					if(*selectedIndex != currentIndex)
						changed = true;
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndPopup();
		}
		NodeEditor::Resume();
		return changed;
	}

	template<typename BaseType, typename... SubTypes>
	class TypeList {
	private:

		template<typename T>
		static std::unique_ptr<BaseType> TryCreate(ordered_json& json, std::string typeName) {
			if (T().GetName() == typeName) {
				return std::unique_ptr<T>(new T(json));
			}
			throw std::exception("Attempting to create unknown type from typelist.");
			return std::unique_ptr<BaseType>();
		}

		template<typename T, typename... types>
		static std::unique_ptr<BaseType> TryCreate(ordered_json& json, std::string typeName, typename std::enable_if<sizeof...(types) >= 1, int>::type = 0) {

			if (T().GetName() == typeName) {
				return std::make_unique<T>(json);
			}
			return TryCreate<types...>(json, typeName);
		}

	public:
		static std::unique_ptr<BaseType> Create(ordered_json& json, std::string typeName) {
			return TryCreate<SubTypes...>(json, typeName);
		}


	private:
		template<typename T>
		static void GetListOfTypeNames(std::vector<std::string>& v) {
			v.push_back(T().GetName());
		}

		template<typename T, typename... types>
		static void GetListOfTypeNames(std::vector<std::string>& v, typename std::enable_if<sizeof...(types) >= 1, int>::type = 0) {
			v.push_back(T().GetName());
			GetListOfTypeNames<types...>(v);
		}

		static bool nameListGenerated;
		static std::vector<std::string> names;

	public:
		static std::vector<std::string>& GetListOfTypeNames() {
			if (nameListGenerated)
				return names;

			GetListOfTypeNames<SubTypes...>(names);
			nameListGenerated = true;
			return names;
		}

	private:
		template<typename T>
		static std::unique_ptr<BaseType> TryCreate(size_t type_idx, size_t idx) {
			if (idx == type_idx) {
				return std::make_unique<T>();
			}
			throw std::exception("Attempting to create unknown type from typelist.");
			return std::unique_ptr<BaseType>();
		}

		template<typename T, typename... types>
		static std::unique_ptr<BaseType> TryCreate(size_t type_idx, size_t idx, typename std::enable_if<sizeof...(types) >= 1, int>::type = 0) {
			if (idx == type_idx) {
				return std::make_unique<T>();
			}
			return TryCreate<types...>(type_idx, idx + 1);
		}

	public:
		static std::unique_ptr<BaseType> Create(size_t type_idx) {
			return TryCreate<SubTypes...>(type_idx, 0);
		}

		static size_t GetTypeIdx(std::string name) {
			auto& n = GetListOfTypeNames();
			return std::distance(n.begin(), std::find(n.begin(), n.end(), name));
		}
	};

}

std::ostream& operator<<(std::ostream& s, const glm::vec3& v);