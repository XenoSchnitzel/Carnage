#pragma once

#include "CoreMinimal.h"

// --- Log category for the state system ---
DECLARE_LOG_CATEGORY_EXTERN(LogStateSystem, Log, All);

// --- Runtime toggle (console variable) ---
extern TAutoConsoleVariable<int32> CVarStateSystemLog;

// --- Convenience macro: logs only if the toggle is enabled ---
#if !UE_BUILD_SHIPPING
#define STATE_LOG(Verbosity, Format, ...) \
    do { if (CVarStateSystemLog.GetValueOnAnyThread() != 0) \
      { UE_LOG(LogStateSystem, Verbosity, TEXT(Format), ##__VA_ARGS__); } } while (0)
#else
  // No logging in Shipping
#define STATE_LOG(Verbosity, Format, ...) do {} while (0)
#endif