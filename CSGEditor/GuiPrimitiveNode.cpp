#include "GuiPrimitiveNode.h"

/*
         GUI PRIMITIVE NODE
*/
#pragma region GUI_PRIMITIVE_NODE
bool GuiPrimitiveNode::DrawPart()
{
    bool updated = false;

    ImGui::PushID((int)id.Get());

    updated |= GuiNode::DrawPart();

    ImGui::PushItemWidth(120);

    typeComboBox.Draw();

    auto primNode = std::static_pointer_cast<PrimitiveNode>(node);
    updated |= primNode->primitive->NodeEditorDraw();

    ImGui::PopItemWidth();
    ImGui::PopID();
    return updated;
}

bool GuiPrimitiveNode::DrawPopups()
{
    bool updated = false;

    ImGui::PushID((int)id.Get());
    GuiNode::DrawPopups();

    auto primNode = std::static_pointer_cast<PrimitiveNode>(node);
    updated |= primNode->primitive->NodeEditorPopup();
    if (typeComboBox.DrawPopup()) {
        updated = true;
        primNode->primitive = PrimitiveTypes::Create(primNode->primitiveIdx);
    }
    ImGui::PopID();
    return updated;
}

std::shared_ptr<GuiPrimitiveNode> GuiPrimitiveNode::CreateFor(std::shared_ptr<PrimitiveNode> primNode, NodeEditor::NodeId id, PinFactoryT pinFactory, RootSetterT rootSetter)
{
    auto node = std::shared_ptr<GuiPrimitiveNode>(new GuiPrimitiveNode(id, primNode, pinFactory, rootSetter));

    auto outPin = pinFactory(node->shared_from_this(), "Output", NodeEditor::PinKind::Output);
    node->outputs.push_back(outPin);

    primNode->guiNode = node;

    return node;
}

GuiPrimitiveNode::GuiPrimitiveNode(NodeEditor::NodeId id, std::shared_ptr<PrimitiveNode> primNode, PinFactoryT pinFactory, RootSetterT rootSetter)
    : GuiNode(id, primNode, pinFactory, rootSetter), typeComboBox("Type")
{

    typeComboBox.Init(&PrimitiveTypes::GetListOfTypeNames(), &primNode->primitiveIdx);
}

std::string GuiPrimitiveNode::GetName()
{
    return "Primitive";
}

ImColor GuiPrimitiveNode::GetColor()
{
    return ImColor(0.0f, 0.5f, 0.0f, 0.5f);
}
#pragma endregion
