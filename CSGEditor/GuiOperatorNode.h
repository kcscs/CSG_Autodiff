#pragma once
#include "GuiNode.h"

struct GuiOperatorNode : public GuiNode {
public:
	virtual std::string GetName() override;
	virtual ImColor GetColor() override;
	virtual bool DrawPart() override;
	virtual bool DrawPopups() override;
	virtual void ReceiveInputFrom(std::shared_ptr<GuiNode> source) override;
	virtual void RemoveInputFrom(std::shared_ptr<GuiNode> source) override;

	bool CheckCircleWithTempNeighbour(std::shared_ptr<GuiNode> tempNeighbour);

	static std::shared_ptr<GuiOperatorNode> CreateFor(std::shared_ptr<OperatorNode> opNode, NodeEditor::NodeId id, PinFactoryT pinFactory, RootSetterT rootSetter);

private:
	GuiOperatorNode(std::shared_ptr<OperatorNode> opNode, NodeEditor::NodeId id, PinFactoryT pinFactory, RootSetterT rootSetter);
	std::vector<std::shared_ptr<Node>> GetDisplayedInputs();

	utils::ComboBoxFix<size_t> typeComboBox;
};

