#pragma once

#include <esp_types.h>

void
taskDelay(uint32_t millis);

[[noreturn]] void
taskSuspend();

void
printHeapInfo(const char* tag, const char* prefix);

void
printStackInfo(const char* tag, const char* prefix);
