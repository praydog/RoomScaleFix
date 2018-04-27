#pragma once

#include <memory>
#include <mutex>

#include "VtableHook.hpp"

#include "common/ITypes.h"
#include "skse64/PapyrusDelayFunctors.h"

// Hooks the HMDNode (contains real position of headset in world and locally)
class NiNodeHook {
public:
    static void Create();

public:
    NiNodeHook();

    void Hook();
    auto& GetHook() {
        return m_hook;
    }

    // how to do an infinite loop 101
    class NiNodeHookLoop : public LatentSKSEDelayFunctor {
    public:
        NiNodeHookLoop()
            : LatentSKSEDelayFunctor{ 0 }
        {
        };

        const char* ClassName() const override {
            return "NiNodeHookLoop";
        }

        UInt32 ClassVersion() const override {
            return 1;
        }

        void Run(VMValue& out) override;
    };


private:
    VtableHook m_hook;

    static void* UpdateTransformHook(class NiNode* node, void* a2);
};

extern std::unique_ptr<NiNodeHook> g_nodeHook;