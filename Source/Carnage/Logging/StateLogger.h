// StateLogger.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/Class.h"        // Cast, TFieldIterator
#include "UObject/UnrealType.h"   // FBoolProperty, FindFProperty
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

// --- Log category for the state system ---
DECLARE_LOG_CATEGORY_EXTERN(LogStateSystem, Log, All);

// --- Runtime toggle (console variable) ---
extern TAutoConsoleVariable<int32> CVarStateSystemLog;

// One macro: pass the context (usually `this`) as first arg.
#if !UE_BUILD_SHIPPING
#define STATE_LOG(Context, Verbosity, Format, ...)                                                  \
do {                                                                                                \
  if (CVarStateSystemLog.GetValueOnAnyThread() != 0) {                                              \
    bool __ok = true;                                                                               \
    const UObject* __obj = Cast<const UObject>(Context);                                            \
    if (__obj) {                                                                                    \
      /* If a component was passed, check its owner actor's flag */                                 \
      if (const UActorComponent* __ac = Cast<const UActorComponent>(__obj)) {                       \
        if (const AActor* __owner = __ac->GetOwner()) { __obj = __owner; }                          \
      }                                                                                             \
      static const FName __flagName(TEXT("bStateLogEnabled"));                                      \
      if (const FBoolProperty* __flag = FindFProperty<FBoolProperty>(__obj->GetClass(), __flagName))\
      {                                                                                             \
        const bool* __val = __flag->ContainerPtrToValuePtr<bool>(__obj);                            \
        __ok = (__val ? *__val : true);                                                             \
      }                                                                                             \
    }                                                                                               \
    if (__ok) { UE_LOG(LogStateSystem, Verbosity, TEXT(Format), ##__VA_ARGS__); }                   \
  }                                                                                                 \
} while (0)
#else
#define STATE_LOG(Context, Verbosity, Format, ...) do {} while (0)
#endif