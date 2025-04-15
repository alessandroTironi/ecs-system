#pragma once 

#include "ProfilingSystem.h"
#include "CycleCounter.h"


#ifdef DEBUG_BUILD
#define SCOPE_CYCLE_COUNTER(Identifier) CycleCounter __counter_##Identifier(#Identifier);
#else 
#define SCOPE_CYCLE_COUNTER(Identifier)
#endif