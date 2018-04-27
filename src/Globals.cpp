#include "Globals.hpp"

IDebugLog g_log("RoomScaleFix.log");
ITimer g_timer;

SKSEMessagingInterface* g_messaging = nullptr;
SKSEObjectInterface* g_object = nullptr;

bool g_allowHookExecution = false;