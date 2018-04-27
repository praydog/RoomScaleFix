#pragma once

#include <windows.h>

#include <map>
#include <set>
#include <cstdint>
#include <string>

#include "common/IDebugLog.h"
#include "common/ITypes.h"

#include "skse64/PluginAPI.h"
#include "skse64/GameEvents.h"
#include "skse64/PapyrusEvents.h"
#include "skse64/PapyrusDelayFunctors.h"
#include "skse64/NiTypes.h"
#include "skse64/GameReferences.h"
#include "skse64/NiNodes.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"

#include "common/IDebugLog.h"
#include "common/ITimer.h"

extern IDebugLog g_log;

extern SKSEMessagingInterface* g_messaging;
extern SKSEObjectInterface* g_object;

extern ITimer g_timer;
extern bool g_allowHookExecution;