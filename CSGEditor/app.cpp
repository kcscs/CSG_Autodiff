#include "app.h"
#include "SDFGenerator.h"
#include "DifferentiatedSDFGenerator.h"
#include "Persistence.h"
#include "exceptions.h"
#include "ShaderLibManager.h"

#include <fstream>
#include <codecvt>
#include <string>
#include <nfd.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <filesystem>

GLuint App::initSphereTracerVao()
{
	sphereTracerVbo.constructMutable(std::vector<glm::vec2>{ {-1, -1}, { 3, -1 }, { -1, 3 } }, GL_STATIC_DRAW);
	sphereTracerVao.addVBO<glm::vec2>(sphereTracerVbo);
	return (GLuint)sphereTracerVao;
}

void App::GenerateShaders(std::shared_ptr<Node> root)
{
	if (root != nullptr) {
		shaderReady = true; // will be overwritten to false in case an error occurs
		SDFGenerator gen;
		try {
			std::string sdf = gen.GenerateFromRoot(root);
			std::ofstream file("Shaders/tmp/sdf.frag", std::ofstream::out);
			file << sdf;
			std::cout << "\nSDF UPDATE:\n" << sdf;
			file.close();

			std::ofstream constantsFile("Shaders/tmp/constants.frag", std::ofstream::out);
			constantsFile << ShaderLibManager::GenerateConstants(enableDerivatives ? derivativeOrder : 0);
			constantsFile.close();

			if (enableDerivatives) {
				std::vector<std::string> func = { "sqrt(x)", "1/(2*sqrt(x))", "-1.0/4 * 1/sqrt(x*x*x)", "3.0/8 * 1/sqrt(x*x*x*x*x)" };
				std::string dsqrt = ShaderLibManager::GenerateChainRuleFunc("dsqrt", func, derivativeOrder);
				func = { "sin(x)", "cos(x)", "-sin(x)", "-cos(x)" };
				std::string dsin = ShaderLibManager::GenerateChainRuleFunc("dsin", func, derivativeOrder);
				func = { "cos(x)", "-sin(x)", "-cos(x)", "sin(x)" };
				std::string dcos = ShaderLibManager::GenerateChainRuleFunc("dcos", func, derivativeOrder);

				std::ofstream chainFuncFile("Shaders/tmp/libgen.frag", std::ofstream::out);
				chainFuncFile << dsqrt << dsin << dcos;
				chainFuncFile.close();

				DifferentiatedSDFGenerator dgen;
				std::string dsdf = dgen.GenerateFromRoot(root);
				std::ofstream file("Shaders/tmp/dsdf.frag", std::ofstream::out);
				file << dsdf;
				std::cout << "\n\nDSDF:\n" << dsdf;
				file.close();
			}

			sphereTracerProgram = std::make_unique<decltype(sphereTracerProgram)::element_type>("RaymarchingProgram");
			*sphereTracerProgram << "Shaders/trace.vert"_vert << "Shaders/tmp/constants.frag"_frag << "Shaders/number.frag"_frag << "Shaders/tmp/primitives_real.frag"_frag << "Shaders/tmp/sdf.frag"_frag;
			if (enableDerivatives)
				*sphereTracerProgram << "Shaders/tmp/libgen.frag"_frag << "Shaders/tmp/primitives_dual.frag"_frag << "Shaders/tmp/dsdf.frag"_frag;
			*sphereTracerProgram << "Shaders/trace.frag"_frag << df::LinkProgram;

			std::string errors = sphereTracerProgram->GetErrors();
			std::cerr << errors;
			transform(errors.begin(), errors.end(), errors.begin(), ::tolower);
			if (errors.find("c5041") != std::string::npos) {
				errorMessageQueue.push("Linking failed: shader too complex.");
			}

			GL_CHECK;
			currentShaderGenException = std::nullopt;
		}
		catch (shader_gen_exception& e) {
			std::cerr << "Failed to generate shader. Is the graph invalid?\n";
			currentShaderGenException = e;
			shaderReady = false;
		}
	}

	editor.ResetDirtyFlag();
	generatorSettingsChanged = false;
	manualGenerateShaders = false;
	redrawNeeded = 2;
}

GLuint App::initDirVao()
{
	dirVbo.constructImmutable(std::vector<Vertex>{ 
		{ {0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f, 0.0f }}, { {1.0f, 0.0f, 0.0f},{1.0f, 0.0f, 0.0f} },
		{ {0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f } }, { {0.0f, 1.0f, 0.0f},{0.0f, 1.0f, 0.0f} },
		{ {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f } }, { {0.0f, 0.0f, 1.0f},{0.0f, 0.0f, 1.0f} }
	});
	dirVao.addVBO<glm::vec3, glm::vec3>(dirVbo);
	return (GLuint)dirVao;
}

GLuint App::initAxesVao()
{
	axesVbo.constructImmutable(std::vector<Vertex>{
		{ {-1.0f, 0.0f, 0.0f}, { 1.0f, 0.6f, 0.6f }}, { {1.0f, 0.0f, 0.0f},{1.0f, 0.6f, 0.6f} },
		{ {0.0f, -1.0f, 0.0f}, { 0.6f, 1.0f, 0.6f } }, { {0.0f, 1.0f, 0.0f},{0.6f, 1.0f, 0.6f} },
		{ {0.0f, 0.0f, -1.0f}, { 0.6f, 0.6f, 1.0f } }, { {0.0f, 0.0f, 1.0f},{0.6f, 0.6f, 1.0f} }
	});
	axesVao.addVBO<glm::vec3, glm::vec3>(axesVbo);
	return (GLuint)axesVao;
}

void App::DrawUI()
{
	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				editor.Clear();
				currentFileName = {};
				shaderReady = false;
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) {
				Save();
			}
			if (ImGui::MenuItem("Save as")) {
				Save(true);
			}
			if (ImGui::MenuItem("Open")) {
				Open();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Jump to selection")) {
				editor.FocusSelection();
			}
			if (ImGui::MenuItem("Fit entire graph")) {
				editor.FocusContent();
			}
			if (ImGui::Checkbox("Axes", &showAxes)) {
				redrawNeeded = 2;
			}
			if (ImGui::Checkbox("Directions", &showDirections)) {
				redrawNeeded = 2;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Derivatives")) {
			ImGui::PushItemWidth(100);
			if (ImGui::Checkbox("Enable", &enableDerivatives)) {
				generatorSettingsChanged = true;
			}
			if (ImGui::InputInt("Order", &derivativeOrder, 1, 5)) {
				if (derivativeOrder < 1)
					derivativeOrder = 1;
				generatorSettingsChanged = true;
			}
			ImGui::PopItemWidth();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Visualization")) {
			ImGui::PushItemWidth(100);
			ImGui::Checkbox("Realtime", &realtime);

			if (enableDerivatives && displayMode != DisplayMode::GAUSSIAN_CURVATURE && displayMode != DisplayMode::MEAN_CURVATURE ||
				enableDerivatives && derivativeOrder > 1)
				if (ImGui::Checkbox("Use automatic differentiation", &useAutoDiff))
					redrawNeeded = 2;

			if (!useAutoDiff || displayMode == DisplayMode::NORMAL_DIFF)
				if (ImGui::InputFloat("eps", &approx_eps, 0.0001f, 0.005f, 5))
					redrawNeeded = 2;

			bool radioPress = false;
			radioPress |= ImGui::RadioButton("Shaded", (int*) & displayMode, (int)DisplayMode::SHADED);

			radioPress |= ImGui::RadioButton("Normals", (int*)&displayMode, (int)DisplayMode::GRADIENT);

			radioPress |= ImGui::RadioButton("Steps", (int*) &displayMode, (int)DisplayMode::STEPS);
			if (enableDerivatives) {
				radioPress |= ImGui::RadioButton("Normal error", (int*)&displayMode, (int)DisplayMode::NORMAL_DIFF);
				if (displayMode == DisplayMode::NORMAL_DIFF) {
					if (ImGui::InputFloat("strength multiplier", &visMultiplier, 0.025f, 0.25f))
						redrawNeeded = 2;
				}
			}

			radioPress |= ImGui::RadioButton("Gaussian curvature", (int*)&displayMode, (int)DisplayMode::GAUSSIAN_CURVATURE);
			if (displayMode == DisplayMode::GAUSSIAN_CURVATURE) {
				if (ImGui::InputFloat("strength multiplier", &visMultiplier, 0.025f, 0.25f))
					redrawNeeded = 2;
			}
			radioPress |= ImGui::RadioButton("Mean curvature", (int*)&displayMode, (int)DisplayMode::MEAN_CURVATURE);
			if (displayMode == DisplayMode::MEAN_CURVATURE) {
				if(ImGui::InputFloat("strength multiplier", &visMultiplier, 0.025f, 0.25f))
					redrawNeeded = 2;
			}
			
			if (radioPress)
				redrawNeeded = 2;
			
			ImGui::PopItemWidth();
			ImGui::EndMenu();
		}

		ImGui::Checkbox("Auto compile", &autoGenerateShaders);
		if (!autoGenerateShaders && ImGui::Button("Compile")) {
			manualGenerateShaders = true;
		}

		ImGui::EndMenuBar();
	}

	if (currentShaderGenException.has_value()) {
		std::string errorText = std::string("ERROR: ") + currentShaderGenException.value().what() + " (click here to focus node)";
		ImVec2 size = ImGui::CalcTextSize(errorText.c_str());
		ImVec2 pos = ImGui::GetCursorPos();
		ImGui::Text(errorText.c_str());
		ImGui::SetCursorPos(pos);
		if (ImGui::InvisibleButton("err_jump", size)) {
			editor.SelectNode(currentShaderGenException.value().source());
		}
	}

	editor.Draw();

	if (isShaderGenerationPending() && !isCompilingPopupOpen) {
		ImGui::OpenPopup("Compiling");
		isCompilingPopupOpen = true;
		ImVec2 center = utils::AddImVec2(utils::ScaleImVec2(ImGui::GetWindowSize(), 0.5f), ImGui::GetWindowPos());
		ImVec2 size = ImVec2(250, 70);
		ImGui::SetNextWindowPos(utils::SubstractImVec2(center, ImVec2(size.x/2, size.y/2)));
		ImGui::SetNextWindowSize(size);
		// this workaround is needed to show the compiling popup before the driver freezes the program
		shaderGenerationCountdown = shaderGenerationDelayFrames;
	}

	if (ImGui::BeginPopupModal("Compiling")) {
		ImGui::Text("Compiling shader...");
		ImGui::Text("This might take a few minutes.");
		if (!isShaderGenerationPending()) {
			ImGui::CloseCurrentPopup();
			isCompilingPopupOpen = false;
		}

		ImGui::EndPopup();
	}

	if (errorMessageQueue.size() > 0 && !isErrorPopupOpen) {
		ImGui::OpenPopup("Error");
		isErrorPopupOpen = true;
		ImVec2 center = utils::AddImVec2(utils::ScaleImVec2(ImGui::GetWindowSize(), 0.5f), ImGui::GetWindowPos());
		ImVec2 size = ImVec2(320, 100);
		ImGui::SetNextWindowPos(utils::SubstractImVec2(center, ImVec2(size.x / 2, size.y / 2)));
		ImGui::SetNextWindowSize(size);
	}

	if (ImGui::BeginPopupModal("Error")) {
		ImGui::Text(errorMessageQueue.front().c_str());
		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
			errorMessageQueue.pop();
			isErrorPopupOpen = false;
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

bool App::isShaderGenerationPending()
{
	return ((editor.IsDirty() || generatorSettingsChanged) && autoGenerateShaders) || manualGenerateShaders;
}

void App::Update()
{
	cam.Update();
	if (redrawKeysDown > 0)
		redrawNeeded = 2;

	if (isShaderGenerationPending()) {
		if (0 >= shaderGenerationCountdown--) {
			manualGenerateShaders = false;
			auto root = editor.GetCurrentRoot();
			GenerateShaders(root);
		}
	}
}

void App::Render()
{
	if (realtime || redrawNeeded > 0) {
		// Draw sphere traced model
		df::Backbuffer << df::Clear(1.0f, 1.0f, 1.0f);
		if (shaderReady) {
			df::Backbuffer << *sphereTracerProgram
				<< "eye_pos" << cam.GetEye()
				<< "inv_view_proj" << cam.GetInverseViewProj()
				<< "view_proj" << cam.GetViewProj()
				<< "to_light" << dirToLight
				<< "display_mode" << (int)displayMode
				<< "vis_multiplier" << visMultiplier
				<< "eps" << approx_eps;
			if (enableDerivatives) // workaround: if autodiff is disabled the shader compiler optimizes out this uniform because it's unused
				df::Backbuffer << *sphereTracerProgram << "use_auto_diff" << (int)useAutoDiff;
			*sphereTracerProgram << sphereTracerVaoArrays;	//Rendering: Ensures that both the vao and program is attached
			GL_CHECK;
			sphereTracerProgram->Render();
		}

		// Draw directional gizmo in upper left corner:
		if (showDirections) {
			glDisable(GL_DEPTH_TEST);
			glm::mat4 gizmoViewProj = cam.GetProj() * glm::lookAt(-cam.GetDir() * 20.0f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			float aspect_ratio = cam.GetSize().x / (float)cam.GetSize().y;
			df::Backbuffer << gizmoProgram
				<< "view_proj" << gizmoViewProj
				<< "shift" << glm::vec2(-1 + 0.15f, 1 - 0.15 * aspect_ratio);
			gizmoProgram << dirVaoArrays;
			GL_CHECK;
			gizmoProgram.Render();
			glEnable(GL_DEPTH_TEST);
		}

		// Draw coordinate axes
		if (showAxes) {
			df::Backbuffer << gizmoProgram
				<< "view_proj" << cam.GetViewProj() * glm::scale(glm::vec3(1e4f, 1e4f, 1e4f))
				<< "shift" << glm::vec2(0, 0);
			gizmoProgram << axesVaoArrays;
			GL_CHECK;
			gizmoProgram.Render();
		}

		if(redrawNeeded > 0)
			redrawNeeded--;
	}
}

void App::Save(bool forceAskFileName)
{
	if (forceAskFileName || !currentFileName.has_value()) {
		nfdchar_t* outPath = NULL;
		nfdresult_t result = NFD_SaveDialog("json", NULL, &outPath);

		if (result == NFD_OKAY) {
			std::cout << "path: " << outPath << "\n";
			currentFileName = outPath;

			Persistence::SaveToJson(editor, outPath);
		}
	}
	else if (currentFileName.has_value()) {
		Persistence::SaveToJson(editor, currentFileName.value());
	}
}

void App::Open()
{
	nfdchar_t* inPath = NULL;
	nfdresult_t result = NFD_OpenDialog("json", NULL, &inPath);

	if (result == NFD_OKAY) {
		std::cout << "path: " << inPath << "\n";
		currentFileName = inPath;

		try {
			Persistence::LoadFromJson(editor, inPath);
		}
		catch (std::exception& e) {
			std::cerr << "Failed to load graph from file. Is the selected file in the correct format?\n";
			errorMessageQueue.push(std::string("Failed to load graph from file.\nIs the selected file in the correct format?\n")+e.what());
			shaderReady = false;
		}
		editor.FocusContent();
	}
}

void AutoLayoutTest(Editor& nodes) {
	auto a = nodes.AddNode(PrimitiveNode::Create());
	auto x = nodes.AddNode(OperatorNode::Create());
	nodes.ConnectNodes(a, x);
	auto b = nodes.AddNode(PrimitiveNode::Create());
	nodes.ConnectNodes(b, x);
	auto c = nodes.AddNode(PrimitiveNode::Create());
	nodes.ConnectNodes(c, x);

	auto y = nodes.AddNode(PrimitiveNode::Create());

	a = nodes.AddNode(PrimitiveNode::Create());
	b = nodes.AddNode(PrimitiveNode::Create());
	auto z = nodes.AddNode(OperatorNode::Create());
	nodes.ConnectNodes(a, z);
	nodes.ConnectNodes(b, z);

	auto i = nodes.AddNode(OperatorNode::Create());
	nodes.ConnectNodes(y, i);
	nodes.ConnectNodes(z, i);

	nodes.AutoArrange();
	nodes.FocusContent();
}

App::App(df::Sample& sam, float aspect) :
	sphereTracerVaoArrays(initSphereTracerVao(), GL_TRIANGLE_STRIP, 3, 0u),
	dirVaoArrays(initDirVao(), GL_LINES, 6u, 0u),
	axesVaoArrays(initAxesVao(), GL_LINES, 6u, 0u),
	gizmoProgram("GizmoProgram")
{
	SDL_GL_SetSwapInterval(1); // enable vsync

	sam.AddHandlerClass(cam, 5); // class callbacks will be called to change its state
	sam.AddHandlerClass(*this, 10);


	gizmoProgram << "Shaders/gizmo.vert"_vert << "Shaders/gizmo.frag"_frag << df::LinkProgram;

	std::cout << gizmoProgram.GetErrors();
	GL_CHECK;

	cam.SetView(glm::vec3(0, 1, 3), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

	ImGui::SetWindowSize("Editor", ImVec2(600, 700));

	glDepthFunc(GL_LEQUAL);
	//AutoLayoutTest(editor);

	std::string vendor((char*)glGetString(GL_VENDOR));
	std::cout << vendor << "\n";

	if (!std::filesystem::exists("Shaders/tmp")) {
		std::filesystem::create_directory("Shaders/tmp");
	}

	ShaderLibManager::GeneratePrimitiveLibs();
}

App::~App()
{
	
}

bool App::HandleMouseMotion(const SDL_MouseMotionEvent& mouse)
{
	if (mouse.state & SDL_BUTTON_LMASK)
		redrawNeeded = 2;
	return false;
}

std::vector<SDL_Keycode> App::redrawKeys = { SDLK_a, SDLK_d, SDLK_w, SDLK_s, SDLK_SPACE };

bool App::HandleKeyDown(const SDL_KeyboardEvent& key)
{
	if (key.keysym.mod & (SDL_Keymod::KMOD_LCTRL | SDL_Keymod::KMOD_RCTRL) && key.keysym.sym == SDLK_s) {
		Save();
		return true;
	}

	if (key.keysym.sym == SDLK_SPACE) {
		dirToLight = -cam.GetDir();
	}

	
	if(!key.repeat)
	for (auto keycode : redrawKeys)
		if (key.keysym.sym == keycode)
			redrawKeysDown++;

	return false;
}


bool App::HandleKeyUp(const SDL_KeyboardEvent& key)
{
	if (key.keysym.mod & (SDL_Keymod::KMOD_LCTRL | SDL_Keymod::KMOD_RCTRL) && key.keysym.sym == SDLK_s) {
		return true;
	}

	for (auto keycode : redrawKeys)
		if (key.keysym.sym == keycode)
			redrawKeysDown--;

	return false;
}