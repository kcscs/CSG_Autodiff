#include "Editor.h"
#include "LayoutCalculatorVisitor.h"
#include "NodeJsonSerializer.h"
#include <ImGui/imgui_internal.h>
#include <unordered_set>
#include <iostream>

Editor::Editor()
{
	context = NodeEditor::CreateEditor();
}

Editor::~Editor()
{
    NodeEditor::DestroyEditor(context);
}

void Editor::Draw()
{
    NodeEditor::SetCurrentEditor(context);

    NodeEditor::Begin("Editor"); // create node editor with id

    // draw all nodes in the graph
    for (auto nodeptr : nodes) { 
        auto& node = *nodeptr.second;
        if (node.Draw()) // returns true if any node property is updated
            SignalPotentialShaderUpdate(); // defer a shader update to the update method in App
    }
    
    // draw links between nodes
    for (auto linkptr : links) { 
        auto& link = *linkptr.second;
        auto from = link.from.lock();
        auto to = link.to.lock();
        assert(from != nullptr);
        assert(to != nullptr);

        NodeEditor::Link(link.id, from->id, to->id);
    }

    // if user is about to create a link or a node by dragging from a pin
    if (NodeEditor::BeginCreate()) {
        NodeEditor::PinId a, b;

        // get the two ends of the link to be created (if there is a link to be created)
        if (NodeEditor::QueryNewLink(&a, &b)) {

            if (a && b) {
                auto from = pins[a.Get()];
                auto to = pins[b.Get()];
                if (from->kind != to->kind && from->kind == NodeEditor::PinKind::Input) {
                    std::swap(from, to);
                    std::swap(a, b);
                }

                if (!AllowConnection(*from, *to)) {
                    NodeEditor::RejectNewItem(ImVec4(1.0f, 0, 0, 1.0f));
                }
                else if (NodeEditor::AcceptNewItem()) {

                    CreateLink(from, to);
                }
            }
        }
    
        // check if the user is about to create a node by dragging a link to an empty spot
        // also get the id of the starting pin
        NodeEditor::PinId id;
        if (NodeEditor::QueryNewNode(&id)) {
            auto pin = pins[id.Get()];
            if (NodeEditor::AcceptNewItem()) {
                NodeEditor::Suspend();
                std::shared_ptr<GuiNode> n;
                if (pin->kind == NodeEditor::PinKind::Input) {
                    n = CreatePrimitiveNode();
                    CreateLink(n->outputs.front(), pin);
                }
                else {
                    n = CreateOperatorNode();
                    CreateLink(pin, n->inputs.front());
                }
                NodeEditor::SetNodePosition(n->id, NodeEditor::ScreenToCanvas(ImGui::GetMousePos()));
                NodeEditor::Resume();
            }
        }
    }
    NodeEditor::EndCreate();

    // check if the user is about to delete something
    if (NodeEditor::BeginDelete()) {

        // query if the object to delete is a link and its id
        NodeEditor::LinkId linkId;
        while (NodeEditor::QueryDeletedLink(&linkId)) {
            if (NodeEditor::AcceptDeletedItem()) {
                DeleteLink(*links[linkId.Get()]);
            }
        }

        // query if the object to delete is a node and its id
        NodeEditor::NodeId nodeId;
        while (NodeEditor::QueryDeletedNode(&nodeId)) {
            if (NodeEditor::AcceptDeletedItem()) {
                DeleteNode(*nodes[nodeId.Get()]);
            }
        }
    }
    NodeEditor::EndDelete();


    // right click context menu
    NodeEditor::Suspend(); // suspend necessary, because of the positioning of the popup
    if (NodeEditor::ShowBackgroundContextMenu()) {
        ImGui::OpenPopup("BackgroundContextMenu");
        ImGui::SetNextWindowPos(ImGui::GetMousePos());
        nextNodePos = NodeEditor::ScreenToCanvas(ImGui::GetMousePos());
    }
    
    if (ImGui::BeginPopup("BackgroundContextMenu")) {

        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Primitive")) {
                auto n = CreatePrimitiveNode();
                NodeEditor::SetNodePosition(n->id, nextNodePos);
            }
            if (ImGui::MenuItem("Operation")) {
                auto n = CreateOperatorNode();
                NodeEditor::SetNodePosition(n->id, nextNodePos);
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
    
    NodeEditor::Resume(); // switch back to canvas coordinates and positioning

    // if there is an external request for selecting nodes, select them
    // this operation had to be deferred to this moment, 
    // since the node editor api only knows about particular node ids while "being drawed"
    if (selectNodeNextDraw.size() > 0) {
        NodeEditor::ClearSelection();
        for (auto& node : selectNodeNextDraw) {
            NodeEditor::SelectNode(node->id, true);
        }
        NodeEditor::NavigateToSelection();
        selectNodeNextDraw = {};
    }

    // check if there is a pending (deferred) focus entire graph operation
    if (focusContentNextDraw) {
        NodeEditor::NavigateToContent();
        focusContentNextDraw = false;
    }

    // check if there is a pending (deferred) focus selection operation
    if (focusSelectionNextDraw) {
        NodeEditor::NavigateToSelection();
        focusSelectionNextDraw = false;
    }

    // handle copy + paste
    if (NodeEditor::BeginShortcut()) {
        if (NodeEditor::AcceptCopy()) {
            CopyToClipBoard(clipBoard);
        }
        if (NodeEditor::AcceptPaste()) {
            PasteClipBoard(clipBoard);
        }

        // future note if you want to implement duplication:
        // NodeEditor::AcceptDuplicate does not recognize ctrl+d due to an internal bug
    }

    NodeEditor::End();
    NodeEditor::SetCurrentEditor(nullptr);
}

void Editor::SignalPotentialShaderUpdate()
{
    dirtyFlag = true;
}

bool Editor::IsDirty()
{
    return dirtyFlag;
}

void Editor::ResetDirtyFlag()
{
    dirtyFlag = false;
}

std::shared_ptr<Node> Editor::GetCurrentRoot()
{
    if (root == nullptr)
        return nullptr;
    return root->node;
}

std::shared_ptr<GuiPrimitiveNode> Editor::CreatePrimitiveNode(std::shared_ptr<PrimitiveNode> primitiveNode)
{
    if (primitiveNode == nullptr)
        primitiveNode = PrimitiveNode::Create();

    auto node = GuiPrimitiveNode::CreateFor(
        primitiveNode,
        idManager.Get(),
        [&](std::shared_ptr<GuiNode> node, std::string name, NodeEditor::PinKind kind) {
            return CreatePinForNode(node, name, kind);
        },
        [&](std::shared_ptr<GuiNode> node) {
            SetRoot(node);
        }
    );

    if (root == nullptr) {
        SetRoot(node);
    }
    nodes.emplace(node->id, node);
    return node;
}

std::shared_ptr<GuiOperatorNode> Editor::CreateOperatorNode(std::shared_ptr<OperatorNode> operationNode)
{
    if (operationNode == nullptr) {
        operationNode = OperatorNode::Create();
    }

    auto node = GuiOperatorNode::CreateFor(
        operationNode,
        idManager.Get(),
        [&](std::shared_ptr<GuiNode> node, std::string name, NodeEditor::PinKind kind) {
            return CreatePinForNode(node, name, kind);
        },
        [&](std::shared_ptr<GuiNode> node) {
            SetRoot(node);
        }
    );

    if (root == nullptr) {
        SetRoot(node);
    }
    nodes.emplace(node->id, node);
    return node;
}

std::shared_ptr<Pin> Editor::CreatePinForNode(std::shared_ptr<GuiNode> node, std::string name, NodeEditor::PinKind kind)
{
    auto pin = std::make_shared<Pin>();
    pin->id = idManager.Get();
    pin->kind = kind;
    pin->node = node;
    pin->name = name;
    pins.emplace(pin->id.Get(), pin);
    return pin;
}

//expects swapping of pins in case of reverse drag to be done by the time it's called
bool Editor::AllowConnection(Pin& from, Pin& to)
{
    if (from.kind == to.kind)
        return false;

    auto fromNode = from.node.lock();
    auto toNode = to.node.lock();

    assert(fromNode != nullptr);
    assert(toNode != nullptr);

    if (fromNode->id == toNode->id)
        return false;

    auto toOpNode = std::dynamic_pointer_cast<GuiOperatorNode>(toNode);
    assert(toOpNode != nullptr); // toNode should allways be an operator as it's the only node with an input
    if (toOpNode->CheckCircleWithTempNeighbour(fromNode))
        return false;

    return true;
}

void Editor::CreateLink(std::shared_ptr<Pin> from, std::shared_ptr<Pin> to)
{
    if (to->link != nullptr) {
        DeleteLink(*to->link);
    }

    if (from->link != nullptr) {
        DeleteLink(*from->link);
    }

    auto link = std::make_shared<Link>();
    link->from = from;
    link->to = to;
    link->id = idManager.Get();

    from->link = to->link = link;
    links.emplace(link->id.Get(), link);

    auto fromNode = from->node.lock();
    auto toNode = to->node.lock();

    assert(fromNode != nullptr);
    assert(toNode != nullptr);

    toNode->ReceiveInputFrom(fromNode);

    SignalPotentialShaderUpdate();
}

void Editor::DeleteLink(Link& link)
{
    auto from = link.from.lock();
    auto to = link.to.lock();

    assert(from != nullptr);
    assert(to != nullptr);

    from->link = nullptr;
    to->link = nullptr;

    auto opNode = std::dynamic_pointer_cast<GuiOperatorNode>(to->node.lock());
    assert(opNode != nullptr);

    auto fromNode = from->node.lock();
    assert(fromNode != nullptr);

    opNode->RemoveInputFrom(fromNode);

    links.erase(link.id.Get());

    SignalPotentialShaderUpdate();
}

void Editor::DeleteNode(GuiNode& node)
{
    for (size_t i = 0; i < node.GetInputPinCount(); ++i) {
        auto linkPtr = node.GetInputPin(i)->link;
        if (linkPtr != nullptr)
            DeleteLink(*linkPtr);
    }

    for (size_t i = 0; i < node.GetOutputPinCount(); ++i) {
        auto linkPtr = node.GetOutputPin(i)->link;
        if (linkPtr != nullptr)
            DeleteLink(*linkPtr);
    }
    bool wasRoot = node.isRoot;
    nodes.erase(node.id.Get()); // this probably frees node (ts just a reference, the shared_ptr was in "nodes")
    if (wasRoot && !nodes.empty())
        SetRoot((*nodes.begin()).second);

    auto _ = std::remove_if(selectNodeNextDraw.begin(), selectNodeNextDraw.end(), [&](NodeHandle handle) { return handle.node->id == node.id; });

    SignalPotentialShaderUpdate();
}

void Editor::SetRoot(std::shared_ptr<GuiNode> newRoot)
{
    if (root != nullptr)
        root->isRoot = false;

    root = newRoot;
    newRoot->isRoot = true;

    SignalPotentialShaderUpdate();
}

std::vector<std::shared_ptr<Node>> Editor::GetRootNodePointers()
{
    std::vector<std::shared_ptr<Node>> roots;
    for (auto kv : nodes) {
        auto n = kv.second;
        if (!n->isOuputOccupied()) {
            roots.push_back(n->node);
        }
    }
    return roots;
}

void Editor::CopyToClipBoard(ClipBoard& cb)
{
    int selectionSize = NodeEditor::GetSelectedObjectCount();
    NodeEditor::NodeId* nodeIds = new NodeEditor::NodeId[selectionSize];
    int count = NodeEditor::GetSelectedNodes(nodeIds, selectionSize);

    std::map<uintptr_t, size_t> clipboardIdx;

    std::vector<std::pair<std::shared_ptr<Node>,ImVec2>> copiedNodes;
    copiedNodes.reserve(count);

    std::vector<std::pair<size_t, size_t>> copiedLinks;

    for (int i = 0; i < count; ++i) {
        auto& nodePtr = nodes[nodeIds[i].Get()]; // ??
        clipboardIdx.emplace(nodeIds[i].Get(), copiedNodes.size());
        copiedNodes.emplace_back(nodePtr->node->clone(), NodeEditor::GetNodePosition(nodePtr->id));
    }

    std::vector<bool> isRoot; // root nodes in selection subgraph
    isRoot.resize(copiedNodes.size(), true);

    for (int i = 0; i < count; ++i) {
        auto& nodePtr = nodes[nodeIds[i].Get()]; // ??

        size_t inputCount = nodePtr->GetInputPinCount();
        for (int j = 0; j < inputCount; ++j) {
            auto inputPinPtr = nodePtr->GetInputPin(j);
            if (inputPinPtr->link != nullptr) {
                auto fromPin = inputPinPtr->link->from.lock();
                assert(fromPin != nullptr);
                auto fromNode = fromPin->node.lock();
                assert(fromNode != nullptr);

                auto fromNodeIdxIt = clipboardIdx.find(fromNode->id.Get());
                if (fromNodeIdxIt != clipboardIdx.end()) {
                    // fromNode is also selected and has been copied to the clipboard
                    size_t fromNodeIdx = (*fromNodeIdxIt).second;
                    copiedLinks.emplace_back(fromNodeIdx, i); // i == clipboardIdx[nodePtr->id.Get()]
                    isRoot[fromNodeIdx] = false; // fromNode is not a root in the selection subgraph, because there is another node taking it's output
                }
            }
        }
    }

    std::vector<NodeHandle> selectionRoots;
    for (int i = 0; i < count; ++i) {
        if (isRoot[i])
            selectionRoots.push_back(nodes[nodeIds[i].Get()]);
    }

    ImGui::SetClipboardText(NodeJsonSerializer::Serialize(selectionRoots).c_str());

    cb = {
        std::move(copiedNodes),
        std::move(copiedLinks),
        NodeEditor::ScreenToCanvas(ImGui::GetWindowPos()),
        NodeEditor::GetCurrentZoom()
    };
    delete[] nodeIds;
}

void Editor::PasteClipBoard(ClipBoard& cb)
{
    NodeEditor::ClearSelection();
    std::vector<NodeHandle> nodeHandles;
    nodeHandles.reserve(cb.nodes.size());

    for (auto nodePositionPair : cb.nodes) {
        float zoomScaling = NodeEditor::GetCurrentZoom() / cb.copyZoom;
        
        auto node = nodePositionPair.first->clone();
        auto& position = nodePositionPair.second;

        auto primNode = std::dynamic_pointer_cast<PrimitiveNode>(node);

        if (primNode != nullptr) {
            auto handle = AddNode(primNode);

            ImVec2 pastePos = utils::AddImVec2(
                NodeEditor::ScreenToCanvas(ImGui::GetWindowPos()),
                utils::ScaleImVec2(
                    utils::SubstractImVec2(
                        position, 
                        cb.copyPos
                        ),
                    zoomScaling)
            );
            pastePos = utils::AddImVec2(pastePos, ImVec2(10, -10));

            SetNodePosition(handle, pastePos);

            nodeHandles.push_back(handle);
            continue;
        }

        auto opNode = std::dynamic_pointer_cast<OperatorNode>(node);
        if (opNode != nullptr) {
            auto handle = AddNode(opNode);

            ImVec2 pastePos = utils::AddImVec2(
                NodeEditor::ScreenToCanvas(ImGui::GetWindowPos()),
                utils::ScaleImVec2(
                    utils::SubstractImVec2(
                        position,
                        cb.copyPos
                    ),
                    zoomScaling)
            );
            pastePos = utils::AddImVec2(pastePos, ImVec2(10, -10));
            
            SetNodePosition(handle, pastePos);

            nodeHandles.push_back(handle);
            continue;
        }
    }

    for (auto& linkIdxPair : cb.links) {
        ConnectNodes(nodeHandles[linkIdxPair.first], nodeHandles[linkIdxPair.second]);
    }

    SelectNodes(nodeHandles);
}


/*
            PUBLIC API FOR MANIPULATING NODES
*/
#pragma region NODE_API

using NodeHandle = Editor::NodeHandle;

NodeHandle Editor::AddNode(std::shared_ptr<PrimitiveNode> node)
{
    auto guiNode = CreatePrimitiveNode(node);
    return NodeHandle(guiNode);
}

NodeHandle Editor::AddNode(std::shared_ptr<OperatorNode> node)
{
    auto guiNode = CreateOperatorNode(node);
    return NodeHandle(guiNode);
}

void Editor::Clear()
{
    std::vector<std::shared_ptr<GuiNode>> nodesToDelete;
    for (auto n : nodes)
        nodesToDelete.push_back(n.second);

    for (auto n : nodesToDelete)
        DeleteNode(*n);

    nodes.clear();
    links.clear();
    pins.clear();
    root = nullptr;

    selectNodeNextDraw = {};
    idManager.Reset();
}


void Editor::ConnectNodes(NodeHandle from, NodeHandle to)
{
    auto outPin = from.node->GetFirstFreeOutput();
    auto inPin = to.node->GetFirstFreeInput();

    if (!AllowConnection(*outPin, *inPin))
        throw std::exception("Invalid connection");

    CreateLink(outPin, inPin);
}

void Editor::SetNodePosition(NodeHandle node, ImVec2 pos)
{
    node.node->SetPositionAtNextDraw(pos);
}

void Editor::SelectNode(NodeHandle node)
{
    selectNodeNextDraw = { node.node };
}

void Editor::SelectNodes(std::vector<NodeHandle> nodes)
{
    selectNodeNextDraw.clear();
    for (auto& handle : nodes) {
        selectNodeNextDraw.push_back(handle.node);
    }
}

void Editor::FocusContent()
{
    focusContentNextDraw = true;
}

void Editor::FocusSelection()
{
    focusSelectionNextDraw = true;
}

void Editor::AutoArrange()
{
    auto roots = GetRootNodePointers();

    LayoutCalculatorVisitor v;
    auto positions = v.CalculateLayout(roots);

    for (auto pair : positions) {
        auto& nodeHandle = pair.first;
        auto& pos = pair.second;
        SetNodePosition(nodeHandle, pos);
    }
}

std::vector<NodeHandle> Editor::GetRootNodes()
{
    auto rootPtrs = GetRootNodePointers();
    std::vector<NodeHandle> roots;
    roots.resize(rootPtrs.size(), std::shared_ptr<GuiNode>(nullptr));

    std::transform(rootPtrs.begin(), rootPtrs.end(), roots.begin(),
        [](std::shared_ptr<Node> ptr) {
            NodeHandle guiNode = ptr->guiNode.lock();
            assert(guiNode.node != nullptr);
            return guiNode;
        });

    return roots;
}

NodeHandle::NodeHandle(std::shared_ptr<GuiNode> node)
{
    this->node = node;
}

#pragma endregion
