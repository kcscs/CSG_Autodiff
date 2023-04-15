#pragma once

#include <ImGui-addons/imgui_node_editor/imgui_node_editor.h>
#include <glm/common.hpp>
#include <vector>
#include <memory>
#include <string>
#include "forward_declarations.h"
#include "Node.h"
#include "GuiNode.h"
#include "GuiPrimitiveNode.h"
#include "GuiOperatorNode.h"
#include "IdManager.h"
#include <unordered_map>

using namespace ax;

class Editor
{
public:
	Editor();
	~Editor();

	/// <summary>
	/// Draws and updates the node editor according to user input.
	/// </summary>
	void Draw();

	/// <summary>
	/// Returns true if the graph has changed since the last shader generation
	/// </summary>
	/// <returns>whether a shader regeneration is needed</returns>
	bool IsDirty();

	/// <summary>
	/// Call this method whenever the shaders have been regenerated
	/// </summary>
	void ResetDirtyFlag();

	/// <summary>
	/// Returns the the node that's currently selected by the user for display.
	/// </summary>
	std::shared_ptr<Node> GetCurrentRoot();

	class NodeHandle {
		friend class Editor;
		friend struct std::hash<NodeHandle>;
		friend struct std::equal_to<NodeHandle>;
		friend class Persistence;
		friend class NodeJsonSerializer;
	public:
		NodeHandle(std::shared_ptr<GuiNode> node);

	private:
		std::shared_ptr<GuiNode> node;
	};

	/// <summary>
	/// Adds a primitive node to the inner graph representation.
	/// </summary>
	/// <param name="node">- the node to add</param>
	/// <returns>a handle to the created inner node</returns>
	NodeHandle AddNode(std::shared_ptr<PrimitiveNode> node);

	/// <summary>
	/// Adds an operator node to the inner graph representation.
	/// </summary>
	/// <param name="node">- the node to add</param>
	/// <returns>a handle to the created inner node</returns>
	NodeHandle AddNode(std::shared_ptr<OperatorNode> node);

	/// <summary>
	/// Clears the entire graph.
	/// </summary>
	void Clear();
	
	/// <summary>
	/// Connects the single output of node "from", to the first free input of node "to".
	/// </summary>
	void ConnectNodes(NodeHandle from, NodeHandle to);

	/// <summary>
	/// Sets the given node's position on the graph editor canvas.
	/// </summary>
	void SetNodePosition(NodeHandle node, ImVec2 pos);

	/// <summary>
	/// Select the node in the graph editor.
	/// </summary>
	void SelectNode(NodeHandle node);

	/// <summary>
	/// Select the nodes in the graph editor.
	/// </summary>
	void SelectNodes(std::vector<NodeHandle> nodes);

	/// <summary>
	/// Change the canvas view window such that it fits the entire graph.
	/// </summary>
	void FocusContent();

	/// <summary>
	/// Change the canvas view window such that it fits the current selection.
	/// </summary>
	void FocusSelection();

	/// <summary>
	/// Rearrange nodes without overlap so that the graph is readable.
	/// </summary>
	void AutoArrange();

	/// <summary>
	/// Get all root nodes of the forest (the graph). Not the one selected by the user for shader generation.
	/// </summary>
	/// <returns></returns>
	std::vector<NodeHandle> GetRootNodes();

private:
	void SignalPotentialShaderUpdate();
	bool dirtyFlag = false;

	std::vector<std::shared_ptr<GuiNode>> selectNodeNextDraw = {};
	bool focusContentNextDraw = false;
	bool focusSelectionNextDraw = false;

	NodeEditor::EditorContext* context;
	std::unordered_map<uintptr_t, std::shared_ptr<GuiNode>> nodes;
	std::unordered_map<uintptr_t, std::shared_ptr<Link>> links;
	std::unordered_map<uintptr_t, std::shared_ptr<Pin>> pins;
	IdManager idManager;
	std::shared_ptr<GuiNode> root = nullptr;

	std::shared_ptr<GuiPrimitiveNode> CreatePrimitiveNode(std::shared_ptr<PrimitiveNode> primitiveNode = nullptr);
	std::shared_ptr<GuiOperatorNode> CreateOperatorNode(std::shared_ptr<OperatorNode> operationNode = nullptr);
	std::shared_ptr<Pin> CreatePinForNode(std::shared_ptr<GuiNode> node, std::string name, NodeEditor::PinKind kind);

	//expects swapping of pins in case of reverse drag to be done by the time it's called
	bool AllowConnection(Pin& from, Pin& to);
	void CreateLink(std::shared_ptr<Pin> from, std::shared_ptr<Pin> to);
	void DeleteLink(Link& link);
	void DeleteNode(GuiNode& node);
	void SetRoot(std::shared_ptr<GuiNode> newRoot);

	std::vector<std::shared_ptr<Node>> GetRootNodePointers();

	ImVec2 nextNodePos;

	struct ClipBoard
	{
		std::vector<std::pair<std::shared_ptr<Node>,ImVec2>> nodes;
		std::vector<std::pair<size_t,size_t>> links;
		ImVec2 copyPos;
		float copyZoom;
	};
	ClipBoard clipBoard;
	void CopyToClipBoard(ClipBoard& cb);
	void PasteClipBoard(ClipBoard& cb);
};

// hash function for nodehandle
namespace std {
	template<>
	struct hash<Editor::NodeHandle> {
		size_t operator()(const Editor::NodeHandle& nh) const {
			return std::hash<std::shared_ptr<GuiNode>>()(nh.node);
		}
	};

	template<>
	struct equal_to<Editor::NodeHandle> {
		bool operator()(const Editor::NodeHandle& a, const Editor::NodeHandle& b) const {
			return a.node == b.node;
		}
	};
}