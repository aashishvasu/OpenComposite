//
// Platform-specific functions
// Created by ZNix on 14/02/2021.
//

#pragma once

typedef void* (*alternativeCoreFactory_t)(const char*, int*);

alternativeCoreFactory_t PlatformGetAlternativeCoreFactory();
