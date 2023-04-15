#include "GuiOperatorNode.h"
#include "CircleCheck.h"
#include <algorithm>

/*
        GUI OPERATOR NODE
*/
#pragma region GUI_OPERATOR_NODE
GuiOperatorNode::GuiOperatorNode(std::shared_ptr<OperatorNode> opNode, NodeEditor::NodeId id, PinFactoryT pinFactory, RootSetterT rootSetter)
    : GuiNode(id, opNode, pinFactory, rootSetter), typeComboBox("Mode")
{
    static std::vector<std::string> labels{ "Union", "Intersect", "Substract" };

    typeComboBox.Init(&OperatorTypes::GetListOfTypeNames(), &opNode->operatorIdx);
}

/// <summary>
/// Derive a vector containing the backend nodes in the same order as their connections' pins are displayed in the editor.
/// Input pins which don't have an assigned link are omitted.
/// </summary>
/// <returns></returns>
std::vector<std::shared_ptr<Node>> GuiOperatorNode::GetDisplayedInputs()
{
    std::vector<std::shared_ptr<Node>> nodes;
    for (auto p : inputs) {
        if (p->link == nullptr)
            continue;

        auto otherEndPin = p->link->from.lock();
        auto otherNode = otherEndPin->node.lock();
        nodes.push_back(otherNode->node);
    }
    return nodes;
}

std::string GuiOperatorNode::GetName()
{
    return "Operator";
}

ImColor GuiOperatorNode::GetColor()
{
    return ImColor(0.5f, 0.0f, 0.0f, 0.5f);
}

bool GuiOperatorNode::DrawPart()
{

    bool updated = false;

    ImGui::PushID((int)id.Get());

    updated |= GuiNode::DrawPart();

    ImGui::PushItemWidth(120);

    typeComboBox.Draw();

    auto opNode = std::static_pointer_cast<OperatorNode>(node);
    updated |= opNode->operatorDescription->NodeEditorDraw();

    ImGui::PopItemWidth();
    ImGui::PopID();
    return updated;
}

bool GuiOperatorNode::DrawPopups()
{
    bool updated = false;
    ImGui::PushID((int)id.Get());
    GuiNode::DrawPopups();

    auto opNode = std::static_pointer_cast<OperatorNode>(node);
    updated |= opNode->operatorDescription->NodeEditorPopup();
    if (typeComboBox.DrawPopup()) {
        updated = true;
        opNode->operatorDescription = OperatorTypes::Create(opNode->operatorIdx);
    }

    ImGui::PopID();
    return updated;
}

void GuiOperatorNode::ReceiveInputFrom(std::shared_ptr<GuiNode> source)
{
    auto opNode = std::static_pointer_cast<OperatorNode>(node);

    // this is necessary to keep the order of inputs in sync (as well as to omit empty pins)
    opNode->UpdateInputs(GetDisplayedInputs());

    if (std::all_of(inputs.begin(), inputs.end(), [](std::shared_ptr<Pin> pin) {return pin->link != nullptr; })) {
        auto newPin = pinFactory(shared_from_this(), "", NodeEditor::PinKind::Input);
        inputs.push_back(newPin);
    }
}

void GuiOperatorNode::RemoveInputFrom(std::shared_ptr<GuiNode> source)
{
    auto opNode = std::static_pointer_cast<OperatorNode>(node);
    opNode->RemoveInput(source->node);
}

bool GuiOperatorNode::CheckCircleWithTempNeighbour(std::shared_ptr<GuiNode> tempNeighbour)
{
    auto opnode = std::static_pointer_cast<OperatorNode>(node);
    return CheckCircleFromNodeWithAddedLink(opnode, tempNeighbour->node);
}

std::shared_ptr<GuiOperatorNode> GuiOperatorNode::CreateFor(std::shared_ptr<OperatorNode> opnode, NodeEditor::NodeId id, PinFactoryT pinFactory, RootSetterT rootSetter)
{
    auto node = std::shared_ptr<GuiOperatorNode>(new GuiOperatorNode(opnode, id, pinFactory, rootSetter));

    auto outPin = pinFactory(node->shared_from_this(), "Output", NodeEditor::PinKind::Output);
    node->outputs.push_back(outPin);

    auto inPin = pinFactory(node->shared_from_this(), "", NodeEditor::PinKind::Input);
    node->inputs.push_back(inPin);

    inPin = pinFactory(node->shared_from_this(), "", NodeEditor::PinKind::Input);
    node->inputs.push_back(inPin);

    opnode->guiNode = node;

    return node;
}
#pragma endregion
