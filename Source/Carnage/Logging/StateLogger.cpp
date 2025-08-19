#include "StateLogger.h"
#include "HAL/IConsoleManager.h"

// Define the category once here
DEFINE_LOG_CATEGORY(LogStateSystem);

// Define the runtime console variable: r.StateSystem.Log (0=off, 1=on)
TAutoConsoleVariable<int32> CVarStateSystemLog(
    TEXT("r.StateSystem.Log"),
    0,
    TEXT("Enable logs for the State System (0=off, 1=on)"),
    ECVF_Cheat
);