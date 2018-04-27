#include "Globals.hpp"
#include "NiNodeHook.hpp"

#include "skse64_common/skse_version.h"

using namespace std;

PluginHandle g_pluginHandle = kPluginHandle_Invalid;

void EventHandler(SKSEMessagingInterface::Message* msg) {
    if (msg == nullptr) {
        return;
    }

    if (msg->type == SKSEMessagingInterface::kMessage_PostLoadGame) {
        // This is where the magic happens
        NiNodeHook::Create();
    }
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info) {
    _MESSAGE("SkyrimVR RoomScaleFix by praydog");

    // populate info structure
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "SkyrimVR RoomScaleFix";
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
