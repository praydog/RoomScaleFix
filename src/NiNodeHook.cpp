#include <list>
#include <numeric>
#include <algorithm>

#include "Globals.hpp"
#include "Structures.hpp"

#include "NiNodeHook.hpp"

using namespace std;

unique_ptr<NiNodeHook> g_nodeHook = nullptr;
mutex g_mutex;

constexpr auto g_updateTransformIndex = 49;

void NiNodeHook::NiNodeHookLoop::Run(VMValue& out) {
    if (g_nodeHook != nullptr) {
        g_nodeHook->Hook();
    }
}

void NiNodeHook::Create() {
    g_timer.Start();
    g_allowHookExecution = false;

    if (g_nodeHook == nullptr) {
        g_nodeHook = make_unique<NiNodeHook>();
    }
    else {
        g_nodeHook->Hook();
    }
}

NiNodeHook::NiNodeHook() {
    Hook();
}

void NiNodeHook::Hook() {
    // just incase some bad voodoo happens
    lock_guard<mutex> lock(g_mutex);

    // infinite loop, check for re-hook every second
    /*struct ScopeGuard { 
        ~ScopeGuard() { g_object->GetDelayFunctorManager().Enqueue(new NiNodeHookLoop{}, 1000); }
    } scopeGuard;*/

    auto myPlayer = (MyPlayer*)*g_thePlayer.GetPtr();

    if (myPlayer == nullptr) {
        return;
    }

    auto hmdNode = myPlayer->hmdNode;

    // dont hook if not ready or already hooked.
    if (hmdNode == nullptr || *(void**)hmdNode == nullptr) {
        return;
    }

    if (m_hook.getInstance().ptr() == hmdNode) {
        return;
    }

    _MESSAGE("Attempting to hook HmdNode");

    // Vtable hook
    if (m_hook.create(hmdNode)) {
        m_hook.hookMethod(g_updateTransformIndex, &NiNodeHook::UpdateTransformHook);

        _MESSAGE("Hooked HmdNode");
    }
}

void* NiNodeHook::UpdateTransformHook(class NiNode* node, void* a2) {
    // this function gets called from multiple threads. for some reason. we might need a different candidate if this starts causing issues.
    lock_guard<mutex> guard(g_mutex);

    auto& hook = g_nodeHook->GetHook();
    auto func = hook.getMethod<decltype(NiNodeHook::UpdateTransformHook)*>(g_updateTransformIndex);

    auto ret = func(node, a2);

    if (!g_allowHookExecution) {
        // After 3 seconds allow this hook to run
        if (g_timer.GetElapsedTime() > 3.0) {
            g_allowHookExecution = true;
        }

        return ret;
    }

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
    auto hmdNode = myPlayer->hmdNode;
    auto followNode = myPlayer->followNode;
    auto roomNode = myPlayer->roomNode;

    // essentialz
    if (hmdNode == nullptr || followNode == nullptr || roomNode == nullptr || proxy == nullptr || proxy->positionContainer == nullptr) {
        return ret;
    }

    static auto lastPos = hmdNode->m_localTransform.pos;

    // smoothest way to do it. local transform does not reset on dirty.
    auto delta = myPlayer->followNode->m_worldTransform.rot * (hmdNode->m_localTransform.pos - lastPos);

    // we dont wanna move really far.
    delta.x = clamp(delta.x, -25.0f, 25.0f);
    delta.y = clamp(delta.y, -25.0f, 25.0f);

    auto len = VectorLength2D(delta);

    // Because this gets called from multiple threads, it may not be updating the transform constantly.
    // So, only do shit when the delta is actually changing.
    if (len > 0.0f) {
        // This is for optional ninja crazy style movement using your headset to move real fast
        /*static list<float> accumulation;

        accumulation.push_back(len);

        auto total = accumulate(accumulation.begin(), accumulation.end(), 0.0f);
        
        if (accumulation.size() > 90) {
            accumulation.pop_front();
        }

        delta.x *= clamp(total * 0.1f, 1.0f, 25.0f);
        delta.y *= clamp(total * 0.1f, 1.0f, 25.0f);*/

        // Tells engine to resync roomNode and others. This is what gets set when rotating the camera with controllers
        myPlayer->hmdFlags |= (uint8_t)HMDFlag::FLAG_DIRTY;
        
        // Update real player position
        proxy->positionContainer->pos.x += delta.x / SKYRIM_SCALE;
        proxy->positionContainer->pos.y += delta.y / SKYRIM_SCALE;
        
        auto newPos = proxy->positionContainer->pos * SKYRIM_SCALE;
        roomNode->m_localTransform.pos.x = hmdNode->m_localTransform.pos.x * -1.0f;
        roomNode->m_localTransform.pos.y = hmdNode->m_localTransform.pos.y * -1.0f;
        roomNode->m_worldTransform.pos.x = newPos.x;
        roomNode->m_worldTransform.pos.y = newPos.y;
        player->pos.x = newPos.x;
        player->pos.y = newPos.y;
    }

    lastPos = hmdNode->m_localTransform.pos;

    return ret;
}
