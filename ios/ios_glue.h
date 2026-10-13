#pragma once
#include <stdbool.h>
// Implemented in ios_glue.cpp
#ifdef __cplusplus
extern "C" {
#endif
void rpcs3_ios_init(const char* data_dir);
bool rpcs3_ios_boot(const char* path);
void rpcs3_ios_stop(void);
void rpcs3_ios_set_view(void* uiview);
void rpcs3_ios_flush_logs(void);
#ifdef __cplusplus
}
#endif
