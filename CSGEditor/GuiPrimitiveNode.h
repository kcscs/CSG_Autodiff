#pragma once
#include "GuiNode.h"

struct GuiPrimitiveNode : public GuiNode {
public:
	virtual std::string GetName() override;
	virtual ImColor GetColor() override;
	virtual bool DrawPart() override;
	virtual bool DrawPopups() override;

	static std::shared_ptr<GuiPrimitiveNode> CreateFor(std::shared_ptr<PrimitiveNode> primNode, NodeEditor::NodeId id, PinFactoryT pinFactory, RootSetterT rootSetter);

private:
	GuiPrimitiveNode(NodeEditor::NodeId id, std::shared_ptr<PrimitiveNode> primNode, PinFactoryT pinFactory, RootSetterT rootSetter);

	utils::ComboBoxFix<size_t> typeComboBox;
};
