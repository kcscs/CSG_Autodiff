#pragma once
#include "forward_declarations.h"
#include <memory>
#include <functional>
#include <optional>
#include "utils.h"
#include "Node.h"

using namespace ax;

/// <summary>
/// Contains information about GUI node pins
/// </summary>
struct Pin {
	/// <summary>
	/// Id of pin in the imgui node editor
	/// </summary>
	NodeEditor::PinId id;
	/// <summary>
	/// Input or output pin
	/// </summary>
	NodeEditor::PinKind kind;

	/// <summary>
	/// Name of pin displayed in the editor
	/// </summary>
	std::string name;

	/// <summary>
	/// The node that contains this pin
	/// </summary>
	std::weak_ptr<GuiNode> node;
	/// <summary>
	/// The link that connects to this pin. (nullptr if there is none)
	/// </summary>
	std::shared_ptr<Link> link;
};

/// <summary>
///	Wraps an instance of Node for displaying it in the editor.
/// </summary>
struct GuiNode : std::enable_shared_from_this<GuiNode> {
	using PinFactoryT = std::function<std::shared_ptr<Pin>(std::shared_ptr<GuiNode>, std::string, NodeEditor::PinKind)>;
	using RootSetterT = std::function<void(std::shared_ptr<GuiNode>)>;

	NodeEditor::NodeId id;
	std::vector<std::shared_ptr<Pin>> inputs;
	std::vector<std::shared_ptr<Pin>> outputs;

	bool useTransform = false;
	bool isRoot = false;

	std::shared_ptr<Node> node;

	/// <summary>
	/// Callback for creating new pins (used on operator nodes)
	/// </summary>
	PinFactoryT pinFactory;
	/// <summary>
	/// Callback for setting the root of the tree in Editor.
	/// </summary>
	RootSetterT rootSetter;


	GuiNode(NodeEditor::NodeId id, std::shared_ptr<Node> node, PinFactoryT pinFactory, RootSetterT rootSetter);

	bool Draw();
	void DrawPins();
	ImVec2 pinStartCursorPos;

	std::shared_ptr<Pin> GetFirstFreeInput();

	/// <summary>
	/// Current implementation ignores if an output is occupied, since an output can go into multiple inputs atm
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<Pin> GetFirstFreeOutput();

	std::optional<int> GetInputPinIndex(NodeEditor::PinId pinId);
	std::optional<int> GetOutputPinIndex(NodeEditor::PinId pinId);

	size_t GetInputPinCount();
	size_t GetOutputPinCount();
	std::shared_ptr<Pin> GetInputPin(size_t idx);
	std::shared_ptr<Pin> GetOutputPin(size_t idx);

	/// <summary>
	/// Assumes node has single output
	/// </summary>
	/// <returns>True if the output pin is occupied, false otherwise.</returns>
	bool isOuputOccupied();

	virtual std::string GetName();
	virtual ImColor GetColor();
	virtual bool DrawPart();
	virtual bool DrawPopups();
	virtual void ReceiveInputFrom(std::shared_ptr<GuiNode> source);
	virtual void RemoveInputFrom(std::shared_ptr<GuiNode> source);

	void SetPositionAtNextDraw(ImVec2 newPos);

private:
	ImVec2 positionToSetAtNextDraw;
	bool needsPositionSetAtNextDraw = false;
	ImVec2 GetAndClearPositionToSetAtDraw();
};

struct Link {
	NodeEditor::LinkId id;
	std::weak_ptr<Pin> from;
	std::weak_ptr<Pin> to;
};