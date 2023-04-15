#pragma once

#include <Dragonfly/editor.h>		 //inlcludes most features
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h>	 //will be replaced

#include "forward_declarations.h"
#include "Editor.h"

#include "exceptions.h"

#include <chrono>
#include <queue>

class App
{
	friend class AppEventHandler;
private:
	df::Camera cam;
	
	// The shader used for displaying the models
	std::unique_ptr<df::ShaderProgramVF> sphereTracerProgram;

	// The shader used for drawing the axes and the direction gizmo
	df::ShaderProgramVF gizmoProgram;

	// These contain a single triangle covering the screen. 
	// Used for running the sphere tracing fragment shader over each screen pixel.
	eltecg::ogl::ArrayBuffer sphereTracerVbo;
	eltecg::ogl::VertexArray sphereTracerVao;
	df::VaoArrays sphereTracerVaoArrays;

	// These contain the vertex data for drawing the direction gizmo in the corner
	eltecg::ogl::ArrayBuffer dirVbo;
	eltecg::ogl::VertexArray dirVao;
	df::VaoArrays dirVaoArrays;

	// These contain the vertex data for the axes
	eltecg::ogl::ArrayBuffer axesVbo;
	eltecg::ogl::VertexArray axesVao;
	df::VaoArrays axesVaoArrays;

	// Class instance handling the editing of the csg graph
	Editor editor;

	// The name (including path) of the file last saved or opened
	std::optional<std::string> currentFileName;

	bool autoGenerateShaders = true;
	bool manualGenerateShaders = false; // serves as an override for regenerating shaders in the next Update() call
	bool isShaderGenerationPending(); // checks if there is a need to regenerate and compile shaders

	int shaderGenerationCountdown = 0; // used for delaying the shader compilation so that the "compiling shader..." modal popup can appear before the freeze.
	const int shaderGenerationDelayFrames = 2;

	bool shaderReady = false; // used for signaling if the shader is compiled and can be used for drawing
	void GenerateShaders(std::shared_ptr<Node> root);

	enum class DisplayMode {SHADED = 0, STEPS = 1, GRADIENT = 2, GAUSSIAN_CURVATURE = 3, MEAN_CURVATURE = 4, NORMAL_DIFF = 5};
	DisplayMode displayMode = DisplayMode::SHADED;
	float visMultiplier = 0.1f; // a multiplier for adjusting the color of curvature, or the strength of displayed errors

	bool showAxes = true; // if true, the axes gizmo will be drawn
	bool showDirections = true; // if true, the direction gizmo will be drawn
	
	// The epsilon used for approximating first and second derivatives
	float approx_eps = 0.01f;

	bool realtime = true; // if false, the displayed image will only be redrawn when there is a change
	bool useAutoDiff = false; // whether to use numeric approximation or automatic differentiation for computing derivatives
	bool enableDerivatives = false; // whether to include the dual library and dual sdf in the generated shader
	int derivativeOrder = 1;

	bool generatorSettingsChanged = false; // signals if any setting that affects shader generation (eg.: derivative order) was changed
	std::optional<shader_gen_exception> currentShaderGenException; // the exception after a failed shader generation attempt, used for displaying error in editor

	bool isCompilingPopupOpen = false;
	bool isErrorPopupOpen = false;
	std::queue<std::string> errorMessageQueue;

	// The direction light is coming from (vector pointing towards light source)
	glm::vec3 dirToLight = glm::normalize(glm::vec3{ 1.5f, 2.0f, 1.0f });

	GLuint initSphereTracerVao();
	GLuint initDirVao();
	GLuint initAxesVao();

	static std::vector<SDL_Keycode> redrawKeys;
	int redrawKeysDown = 0;
	int redrawNeeded = 0;

public:
	void DrawUI();
	void Update();
	void Render();

	void Save(bool forceAskFileName = false);
	void Open();

	bool HandleKeyDown(const SDL_KeyboardEvent& key);
	bool HandleKeyUp(const SDL_KeyboardEvent& key);
	bool HandleMouseMotion(const SDL_MouseMotionEvent& mouse);

	App(df::Sample& sam, float aspect);
	~App();
};

// vertex data for gizmos
struct Vertex {
	glm::vec3 pos;
	glm::vec3 col;
};