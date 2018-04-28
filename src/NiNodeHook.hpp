#pragma once

#include <memory>
#include <mutex>

#include "VtableHook.hpp"

#include "common/ITypes.h"
#include "skse64/PapyrusDelayFunctors.h"

#include "Structures.hpp"

// Hooks the HMDNode (contains real position of headset in world and locally)
// I thought about using the NiNode update event in SKSE, but it doesn't seem to do anything (unless I was using it wrong?)
class NiNodeHook {
public:
    using UpdateTransformCallback = std::function<void(NiNode*, NiNodeHook&, bhkCharProxyController*, const MyNiPoint3&)>;

    static void Create(const std::string& name, UpdateTransformCallback callback);
    static void Remove();

public:
    NiNodeHook() = delete;
    NiNodeHook(const std::string& name, UpdateTransformCallback callback);

    virtual ~NiNodeHook() = default;

    void Hook();

    auto& GetHook() {
        return m_hook;
    }

    const auto& GetLastPos() const {
        return m_lastPos;
    }

    const auto& GetLastDelta() const {
        return m_lastDelta;
    }

    const auto& GetNodeName() const {
        return m_nodeName;
    }

    void* UpdateTransformHook(class NiNode* node, void* a2);

private:
    VtableHook m_hook;
    std::string m_nodeName;

    MyNiPoint3 m_lastPos;
    MyNiPoint3 m_lastDelta;

    UpdateTransformCallback m_callback;

    class NiNode* FindNode();
    class NiNode* FindNodeFromChildren(class NiAVObject* obj, const std::string& prefix = "");

    static void* UpdateTransformHookInternal(class NiNode* node, void* a2);
};
