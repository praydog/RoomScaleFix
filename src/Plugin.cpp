#include "Globals.hpp"
#include "NiNodeHook.hpp"

#include "skse64_common/skse_version.h"

using namespace std;

void UpdatePlayerPos(NiNode* node, MyNiPoint3& delta, bhkCharProxyController* proxy) {
    auto player = *g_thePlayer.GetPtr();
    auto myPlayer = (MyPlayer*)player;

    // we dont wanna move really far.
    delta.x = clamp(delta.x, -25.0f, 25.0f);
    delta.y = clamp(delta.y, -25.0f, 25.0f);

    // Tells engine to resync roomNode and others. This is what gets set when rotating the camera with controllers
    myPlayer->hmdFlags |= (uint8_t)HMDFlag::FLAG_DIRTY;

    // Update real player position
    proxy->positionContainer->pos.x += delta.x / SKYRIM_SCALE;
    proxy->positionContainer->pos.y += delta.y / SKYRIM_SCALE;

    auto newPos = proxy->positionContainer->pos * SKYRIM_SCALE;

    player->pos.x = newPos.x;
    player->pos.y = newPos.y;
}

// Fancy movement with controllers
void WandHandler(NiNode* node, NiNodeHook& hook, bhkCharProxyController* proxy, const MyNiPoint3& localDelta) {
    auto myPlayer = (MyPlayer*)*g_thePlayer.GetPtr();

    MyNiPoint3 forward{ myPlayer->hmdNode->m_worldTransform.rot * NiPoint3{ 0.0f, 1.0f, 0.0f } };

    // average
    auto len = localDelta.Length2D() + hook.GetLastDelta().Length2D();
    len *= 0.5f;

    MyNiPoint3 delta{ myPlayer->followNode->m_worldTransform.rot * localDelta };
    delta.x *= clamp(len, 0.0f, 10.0f);
    delta.y *= clamp(len, 0.0f, 10.0f);

    delta.x *= fabs(forward.x);
    delta.y *= fabs(forward.y);

    UpdatePlayerPos(node, delta, proxy);
}

// Updates player position/hitbox exactly with 2D headset position
void HMDHandler(NiNode* node, NiNodeHook& hook, bhkCharProxyController* proxy, const NiPoint3& localDelta) {
    auto player = *g_thePlayer.GetPtr();
    auto myPlayer = (MyPlayer*)*g_thePlayer.GetPtr();

    MyNiPoint3 delta{ myPlayer->followNode->m_worldTransform.rot * localDelta };

    UpdatePlayerPos(node, delta, proxy);

    auto roomNode = myPlayer->roomNode;

    if (roomNode != nullptr) {
        roomNode->m_localTransform.pos.x = node->m_localTransform.pos.x * -1.0f;
        roomNode->m_localTransform.pos.y = node->m_localTransform.pos.y * -1.0f;
        roomNode->m_worldTransform.pos.x = player->pos.x;
        roomNode->m_worldTransform.pos.y = player->pos.y;
    }
}

void EventHandler(SKSEMessagingInterface::Message* msg) {
    if (msg == nullptr) {
        return;
    }

    // Remove hooks. Causes crash sometimes otherwise
    if (msg->type == SKSEMessagingInterface::kMessage_PreLoadGame) {
        NiNodeHook::Remove();
    }

    if (msg->type == SKSEMessagingInterface::kMessage_PostLoadGame || msg->type == SKSEMessagingInterface::kMessage_InputLoaded) {
        // This is where the magic happens
        NiNodeHook::Create("HmdNode", HMDHandler);
        NiNodeHook::Create("RightWandNode", WandHandler);
        NiNodeHook::Create("LeftWandNode", WandHandler);
    }
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info) {
    _MESSAGE("SkyrimVR RoomScaleFix by praydog");

    // populate info structure
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "RoomScaleFix";
    info->version = 1;

    // store plugin handle so we can identify ourselves later
    g_pluginHandle = skse->GetPluginHandle();

    if (skse->isEditor)
    {
        _MESSAGE("Loaded in editor, marking as incompatible");
        return false;
    }

    if (skse->runtimeVersion != RUNTIME_VR_VERSION_1_3_64) {
        _MESSAGE("Unsupported version! This version of RoomScaleFix only supports SkyrimVR 1.3.64!");
        return false;
    }

    // supported runtime version
    return true;
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSEInterface * skse) {
    _MESSAGE("SkyrimVR RoomScaleFix Loaded");

    g_messaging = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging);

    if (g_messaging == nullptr) {
        _MESSAGE("Failed to get Messaging interface");
        return false;
    }

    g_object = (SKSEObjectInterface*)skse->QueryInterface(kInterface_Object);

    if (g_object == nullptr) {
        _MESSAGE("Failed to get Object interface");
        return false;
    }

    if (!g_branchTrampoline.Create(1024 * 64))
    {
        _ERROR("Failed to create branch trampoline");
        return false;
    }

    g_messaging->RegisterListener(skse->GetPluginHandle(), "SKSE", &EventHandler);

    return true;
}
