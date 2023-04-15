#include "GuiNode.h"
#include <ImGui/imgui_internal.h>


/*
        GUI NODE
*/
#pragma region GUI_NODE

bool GuiNode::isOuputOccupied()
{
    return outputs.front()->link != nullptr;
}

std::string GuiNode::GetName()
{
    return "Node";
}

ImColor GuiNode::GetColor()
{
    return ImColor(0.0f,0.0f,0.5f,0.5f);
}

bool GuiNode::DrawPart()
{
    bool changed = false;


    pinStartCursorPos = ImGui::GetCursorScreenPos();
    float lineHeight = ImGui::GetTextLineHeightWithSpacing();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + lineHeight * std::max(inputs.size(), outputs.size())); // skip space left for pins

    ImGui::PushID((int)id.Get());
    ImGui::Checkbox("Show transform", &useTransform);
    if (useTransform) {
        ImGui::PushItemWidth(120);
        changed |= utils::InputVec3("translate", node->translate, 2);
        changed |= utils::InputVec3("rotate", node->rotate, 2);
        changed |= ImGui::InputFloat("scale", &node->scale);
        changed |= ImGui::InputFloat("offset", &node->radius);
        ImGui::PopItemWidth();
    }
    ImGui::PopID();

    return changed;
}

bool GuiNode::DrawPopups()
{
    return false;
}

void GuiNode::ReceiveInputFrom(std::shared_ptr<GuiNode> source) // is overridden by GuiOperatorNode
{
    assert(false); // should not be called on GuiPrimitiveNode
}

void GuiNode::RemoveInputFrom(std::shared_ptr<GuiNode> source) // is overridden by GuiOperatorNode
{
    assert(false); // should not be called on GuiPrimitiveNode
}

void GuiNode::SetPositionAtNextDraw(ImVec2 newPos)
{
    positionToSetAtNextDraw = newPos;
    needsPositionSetAtNextDraw = true;
}

ImVec2 GuiNode::GetAndClearPositionToSetAtDraw()
{
    needsPositionSetAtNextDraw = false;
    return positionToSetAtNextDraw;
}

GuiNode::GuiNode(NodeEditor::NodeId id, std::shared_ptr<Node> node, PinFactoryT pinFactory, RootSetterT rootSetter) 
    : id(id), node(node), pinFactory(pinFactory), rootSetter(rootSetter)
{
}

bool GuiNode::Draw()
{
    bool updated = false;

    NodeEditor::BeginNode(id);
    float headerHeight = ImGui::GetCursorPosY();
    ImGui::Text(GetName().c_str());
    headerHeight = ImGui::GetCursorPosY() - headerHeight + NodeEditor::GetStyle().NodePadding.x;
    ImGui::Spacing();

    updated |= DrawPart();

    DrawPins();

    NodeEditor::EndNode();

    if (isRoot) {
        ImVec2 tl = NodeEditor::GetNodePosition(id);
        ImVec2 br = utils::AddImVec2(tl, NodeEditor::GetNodeSize(id));
        NodeEditor::GetNodeBackgroundDrawList(id)->AddRectFilled(
            tl, br,
            ImColor(50, 10, 170),
            NodeEditor::GetStyle().NodeRounding
        );
    }



    float border = NodeEditor::GetStyle().NodeBorderWidth;
    ImVec2 topLeft = utils::AddImVec2(NodeEditor::GetNodePosition(id), ImVec2(border, border));
    ImVec2 bottomRight = utils::AddImVec2(topLeft, ImVec2(NodeEditor::GetNodeSize(id).x - border*2, headerHeight - border));
    NodeEditor::GetNodeBackgroundDrawList(id)->AddRectFilled(topLeft, bottomRight, GetColor(), NodeEditor::GetStyle().NodeRounding, ImDrawCornerFlags_::ImDrawCornerFlags_Top);

    float makeRootBtnSize = 12;
    topLeft = utils::AddImVec2(
        NodeEditor::GetNodePosition(id),
        ImVec2(NodeEditor::GetNodeSize(id).x - NodeEditor::GetStyle().NodePadding.x-border-makeRootBtnSize, NodeEditor::GetStyle().NodePadding.y+border-1)
    );

    bottomRight = utils::AddImVec2(topLeft, ImVec2(makeRootBtnSize, makeRootBtnSize));
    ImColor rootBtnColor;
    if (isRoot) 
        rootBtnColor = ImColor(230, 210, 20);
    else 
        rootBtnColor = ImColor(0, 0, 255, 255);
    

    NodeEditor::GetNodeBackgroundDrawList(id)->AddRectFilled(topLeft, bottomRight, rootBtnColor, 5);
    
    ImGui::SetCursorScreenPos(topLeft);
    ImGui::PushID((int)id.Get());
    if (ImGui::InvisibleButton("", ImVec2(makeRootBtnSize, makeRootBtnSize))) {
        rootSetter(shared_from_this());
    }

    if (needsPositionSetAtNextDraw)
        NodeEditor::SetNodePosition(id, GetAndClearPositionToSetAtDraw());

    ImGui::PopID();

    updated |= DrawPopups();
    return updated;
}

void GuiNode::DrawPins()
{
    ImGui::SetCursorScreenPos(pinStartCursorPos);


    auto inputIter = inputs.begin();
    auto outputIter = outputs.begin();

    auto drawInputPin = [](Pin& pin) {
        NodeEditor::BeginPin(pin.id, pin.kind);
        auto drawlist = ImGui::GetWindowDrawList();
        auto cursorpos = ImGui::GetCursorScreenPos();
        ImVec2 top = ImVec2(cursorpos.x + 3.0f, cursorpos.y + 3.0f);
        ImVec2 bottom = ImVec2(cursorpos.x + 3.0f, cursorpos.y + 11.0f);
        ImVec2 right = ImVec2(cursorpos.x + 13.0f, cursorpos.y + 7.0f);
        ImColor col(255, 255, 0);
        drawlist->AddTriangle(top, right, bottom, col, 2.0f);
        ImGui::ItemSize(ImVec2(14, 14));
        NodeEditor::EndPin();
        ImGui::SameLine();
        ImGui::Text(pin.name.c_str());
    };

    auto drawOutputPin = [&](Pin& pin) {
        float size = ImGui::CalcTextSize("Output").x + 32.0f;
        ImGui::SetCursorScreenPos(ImVec2(NodeEditor::GetNodePosition(id).x + NodeEditor::GetNodeSize(id).x - size, pinStartCursorPos.y)); // right align output pin (hard coded)
        ImGui::Text(pin.name.c_str());
        ImGui::SameLine();
        NodeEditor::BeginPin(pin.id, pin.kind);
        auto drawlist = ImGui::GetWindowDrawList();
        auto cursorpos = ImGui::GetCursorScreenPos();
        ImVec2 top = ImVec2(cursorpos.x + 3.0f, cursorpos.y + 3.0f);
        ImVec2 bottom = ImVec2(cursorpos.x + 3.0f, cursorpos.y + 11.0f);
        ImVec2 right = ImVec2(cursorpos.x + 13.0f, cursorpos.y + 7.0f);
        ImColor col(255, 255, 0);
        drawlist->AddTriangle(top, right, bottom, col, 2.0f);
        ImGui::ItemSize(ImVec2(14, 14));
        NodeEditor::EndPin();
    };

    float maxLeftWidth = ImGui::GetCursorPosX();

    while (inputIter != inputs.end() && outputIter != outputs.end()) {

        auto& pin = **inputIter;
        drawInputPin(pin);

        ImGui::SameLine();

        maxLeftWidth = std::max(maxLeftWidth, ImGui::GetCursorPosX());

        auto& opin = **outputIter;
        drawOutputPin(opin);

        inputIter++;
        outputIter++;
    }

    while (inputIter != inputs.end()) {
        auto& pin = **inputIter;
        drawInputPin(pin);
        inputIter++;
    }

    while (outputIter != outputs.end()) {
        auto& pin = **outputIter;
        ImGui::SetCursorPosX(maxLeftWidth);
        drawOutputPin(pin);
        outputIter++;
    }
}

std::shared_ptr<Pin> GuiNode::GetFirstFreeInput()
{
    auto freePinIter = std::find_if(inputs.begin(), inputs.end(), [](std::shared_ptr<Pin> pin) {return pin->link == nullptr; });
    if (freePinIter != inputs.end())
        return *freePinIter;
    return nullptr;
}

std::shared_ptr<Pin> GuiNode::GetFirstFreeOutput()
{
    if (outputs.size() > 0)
        return outputs.front();
    return nullptr;
}

std::optional<int> GuiNode::GetInputPinIndex(NodeEditor::PinId pinId)
{
    for (int i = 0; i < inputs.size(); ++i) {
        if (inputs[i]->id == pinId)
            return i;
    }
    return std::nullopt;
}

std::optional<int> GuiNode::GetOutputPinIndex(NodeEditor::PinId pinId)
{
    for (int i = 0; i < outputs.size(); ++i) {
        if (outputs[i]->id == pinId)
            return i;
    }
    return std::nullopt;
}

size_t GuiNode::GetInputPinCount()
{
    return inputs.size();
}

size_t GuiNode::GetOutputPinCount()
{
    return outputs.size();
}

std::shared_ptr<Pin> GuiNode::GetInputPin(size_t idx)
{
    while (idx >= inputs.size()) {
        inputs.push_back(pinFactory(shared_from_this(), "", NodeEditor::PinKind::Input));
    }

    return inputs.at(idx);
}

std::shared_ptr<Pin> GuiNode::GetOutputPin(size_t idx)
{
    return outputs.at(idx);
}

#pragma endregion
