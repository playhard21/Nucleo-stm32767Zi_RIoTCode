#pragma once

#ifdef ENABLE_DEBUG
#define DEBUG(...) Serial.print(__VA_ARGS__)
#define DEBUGLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG(...) (void)0
#define DEBUGLN(...) (void)0
#endif
