#pragma once

#include <windows.h>

#include <map>
#include <set>
#include <cstdint>
#include <string>
#include <algorithm>

#include "common/IDebugLog.h"
#include "common/ITypes.h"


#include "skse64/PluginManager.h"
#include "skse64/GameEvents.h"
#include "skse64/PapyrusEvents.h"
#include "skse64/PapyrusDelayFunctors.h"
#include "skse64/NiTypes.h"
#include "skse64/GameReferences.h"
#include "skse64/NiNodes.h"
#include "skse64/GameRTTI.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"

#include "common/IDebugLog.h"
#include "common/ITimer.h"


extern PluginHandle g_pluginHandle;

extern SKSEMessagingInterface* g_messaging;
extern SKSEObjectInterface* g_object;
