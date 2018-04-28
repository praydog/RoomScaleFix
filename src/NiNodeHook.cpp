#include <list>
#include <numeric>
#include <algorithm>
#include <map>

#include "Globals.hpp"
#include "Structures.hpp"

#include "NiNodeHook.hpp"

using namespace std;

unordered_map<string, unique_ptr<NiNodeHook>> g_nodeHooks;
mutex g_mutex;
constexpr auto g_updateTransformIndex = 49;

void NiNodeHook::Create(const std::string& name, UpdateTransformCallback callback) {
    if (g_nodeHooks[name] == nullptr) {
        g_nodeHooks[name] = make_unique<NiNodeHook>(name, callback);
    }
    
    g_nodeHooks[name]->Hook();
}

void NiNodeHook::Remove() {
    for (auto& hook : g_nodeHooks) {
        hook.second.reset();
    }

    g_nodeHooks.clear();
}

NiNodeHook::NiNodeHook(const std::string& name, UpdateTransformCallback callback)
    : m_nodeName{ name },
    m_lastPos{},
    m_callback{ callback }
{
    // Don't do hook in here because the pointer may not be assigned by the time hook returns
    // At least, I think that will happen.
}

void NiNodeHook::Hook() {
    // just incase some bad voodoo happens
    lock_guard<mutex> lock(g_mutex);

    auto hookNode = FindNode();

    // dont hook if not ready or already hooked.
    if (hookNode == nullptr || *(void**)hookNode == nullptr) {
        return;
    }

    _MESSAGE("Attempting to hook %s", hookNode->m_name);

    // Vtable hook. Recreate on every load
    if (m_hook.create(hookNode)) {
        m_hook.hookMethod(g_updateTransformIndex, &NiNodeHook::UpdateTransformHookInternal);

        m_lastPos = {};
        m_lastDelta = {};

        _MESSAGE("Hooked %s", m_nodeName.c_str());

        // beep boop
        MessageBeep(0);
    }
    else {
        _MESSAGE("Failed to hook %s", m_nodeName.c_str());
    }
}

class NiNode* NiNodeHook::FindNode() {
    _MESSAGE("Checking player");

    auto player = *g_thePlayer.GetPtr();
    auto myPlayer = (MyPlayer*)player;

    if (player == nullptr) {
        return nullptr;
    }

    return FindNodeFromChildren((NiAVObject*)myPlayer->playerWorldNode);
}

class NiNode* NiNodeHook::FindNodeFromChildren(NiAVObject* obj, const std::string& prefix) {
    if (obj == nullptr) {
        return nullptr;
    }

    auto normalNode = DYNAMIC_CAST(obj, NiAVObject, NiNode);
    auto node = (MyNiNode*)normalNode;

    // cast failed
    if (normalNode == nullptr) {
        return nullptr;
    }

    if (normalNode->m_name != nullptr && m_nodeName == normalNode->m_name) {
        return (NiNode*)node;
    }

    if (normalNode->m_name != nullptr) {
        _MESSAGE("%s%s (0x%p)", prefix.c_str(), normalNode->m_name, normalNode);
    }

    if (node->children.m_data == nullptr || node->children.m_size == 0 || node->children.m_arrayBufLen == 0) {
        return nullptr;
    }

    for (auto i = 0; i < node->children.m_emptyRunStart; ++i) {
        auto child = node->children.m_data[i];

        if (child == nullptr) {
            continue;
        }

        if (auto childNode = FindNodeFromChildren(child, prefix + " ")) {
            return childNode;
        }
    }

    return nullptr;
}

void* NiNodeHook::UpdateTransformHookInternal(class NiNode* node, void* a2) {
    // this function gets called from multiple threads. for some reason. we might need a different candidate if this starts causing issues.
    lock_guard<mutex> guard(g_mutex);

    return g_nodeHooks[node->m_name]->UpdateTransformHook(node, a2);
}

void* NiNodeHook::UpdateTransformHook(NiNode* node, void* a2) {
    auto func = m_hook.getMethod<void* (__thiscall*)(NiNode*, void*)>(g_updateTransformIndex);
    auto ret = func(node, a2);

    auto player = *g_thePlayer.GetPtr();
    auto myPlayer = (MyPlayer*)player;

    if (myPlayer == nullptr || player->processManager == nullptr || player->processManager->middleProcess == nullptr || player->processManager->middleProcess->unk250 == nullptr) {
        return ret;
    }

    // Usually gets set during cutscenes.
    if ((myPlayer->hmdFlags & (uint8_t)HMDFlag::FLAG_ALLOW_UPDATE) == 0) {
        return ret;
    }

    auto proxy = (bhkCharProxyController*)player->processManager->middleProcess->unk250;
    auto roomNode = myPlayer->roomNode;

    // essentialz
    if (roomNode == nullptr || proxy == nullptr || proxy->positionContainer == nullptr) {
        return ret;
    }

    // smoothest way to do it. local transform does not reset on dirty.
    MyNiPoint3 localDelta{ node->m_localTransform.pos - m_lastPos };
    auto len = localDelta.Length2D();

    // Because this gets called from multiple threads, it may not be updating the transform constantly.
    // So, only do shit when the delta is actually changing.
    if (len > 0.0f) {
        if (m_callback) {
            m_callback(node, *this, proxy, localDelta);
        }
    }

    m_lastPos = node->m_localTransform.pos;
    m_lastDelta = localDelta;

    return ret;
}

